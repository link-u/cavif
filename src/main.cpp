#include <iostream>

#include "../external/clipp/include/clipp.h"
#include "Image.hpp"
#include "PNGReader.hpp"
#include "AVIFBuilder.hpp"
#include <aom/aom_encoder.h>
#include <aom/aom_codec.h>
#include <aom/aomcx.h>
#include <libyuv.h>
#include <avif/av1/Parser.hpp>
#include <avif/util/FileLogger.hpp>
#include <avif/util/File.hpp>
#include <avif/FileBox.hpp>
#include <avif/Writer.hpp>

namespace {

// FIXME(ledyba-z): remove this function when the C++20 comes.
bool endsWidh(std::string const& target, std::string const& suffix) {
  if(target.size() < suffix.size()) {
    return false;
  }
  return target.substr(target.size()-suffix.size()) == suffix;
}

std::string basename(std::string const& path) {
  auto pos = path.find_last_of('/');
  if(pos == std::string::npos) {
    return path;
  }
  return path.substr(pos+1);
}

}

int main(int argc, char** argv) {
  avif::util::FileLogger log(stdout, stderr, avif::util::Logger::Level::DEBUG);

  std::string inputFilename = {};
  std::string outputFilename = {};
  aom_codec_ctx_t codec = {};
  aom_codec_enc_cfg cfg = {};
  aom_codec_iface_t* av1codec = aom_codec_av1_cx();
  if(!av1codec) {
    log.fatal("failed to get AV1 encoder.");
  }
  aom_codec_enc_config_default(av1codec, &cfg, 0);
  {
    using namespace clipp;
    auto cli = (
        required("-i", "--input") & value("input.{png, bmp}", inputFilename),
        required("-o", "--output") & value("output.avif", outputFilename),
        option("--cfg.encoder_cfg.disable_cdef").set(cfg.encoder_cfg.disable_cdef, 1u),
        option("--cfg.monochrome").set(cfg.monochrome, 1u)
    );

    if(!parse(argc, argv, cli)) {
      std::cerr << make_man_page(cli, basename(std::string(argv[0])));
      return -1;
    }

    if(inputFilename == outputFilename) {
      std::cerr << make_man_page(cli, basename(std::string(argv[0])));
      return -1;
    }
  }

  // decoding input image
  img::Image srcImage;
  if(endsWidh(inputFilename, ".png")) {
    srcImage = img::PNGReader(inputFilename).read();
  } else if(endsWidh(inputFilename, ".bmp")) {

  } else {
    log.fatal("please give png or bmp file for input");
  }
  uint32_t const width = srcImage.width();
  uint32_t const height = srcImage.height();

  // converting RGB(A) -> I420
  // FIXME(ledyba-z): how about alpha channels?
  std::vector<uint8_t> staging;
  uint32_t const stagingBytesPerPiexl = 4;
  uint32_t const stagingStride = width * stagingBytesPerPiexl;

  switch(srcImage.type()) {
    case img::Image::Type::BGR: {
      std::vector<uint8_t> abgr;
      size_t abgrStride = 4 * width;
      abgr.resize(abgrStride * height);
      // BGR24ToABGRが無かった
      libyuv::RGB24ToARGB(srcImage.data().data(), srcImage.stride(), abgr.data(), abgrStride, width, height);

      // BGR24ToARGBが無かった
      staging.resize(stagingStride * height);
      libyuv::ABGRToARGB(abgr.data(), abgrStride, staging.data(), stagingStride, width, height);
      break;
    }
    case img::Image::Type::ABGR: {
      staging = srcImage.data();
      staging.resize(stagingStride * height);
      libyuv::ABGRToARGB(srcImage.data().data(), srcImage.stride(), staging.data(), stagingStride, width, height);
      break;
    }
  }

  // initialize encoder
  cfg.g_w = width;
  cfg.g_h = height;
  cfg.encoder_cfg.init_by_cfg_file = 1;
  cfg.encoder_cfg.disable_cdef = 1;
  cfg.encoder_cfg.disable_dual_filter = 1;
  cfg.monochrome = 0;
  cfg.g_profile = 0;
  cfg.full_still_picture_hdr = 0;

  if(AOM_CODEC_OK != aom_codec_enc_init(&codec, av1codec, &cfg, 0)) {
    log.fatal("Failed to initialize encoder.");
  }

  aom_codec_control(&codec, AV1E_SET_DENOISE_NOISE_LEVEL, 1);

  aom_image_t img;
  // FIXME: read validate_img() function.
  // https://github.com/link-u/libaom/blob/d0b3d306aebb5ef6cc89f49f56dd7adaee41f696/av1/av1_cx_iface.c#L532
  //aom_img_alloc(&img, AOM_IMG_FMT_I420, width, height, 1);
  //libyuv::ARGBToI420(staging.data(), statingStride, img.planes[0], img.stride[0], img.planes[1], img.stride[1], img.planes[2], img.stride[2], width, height);
  aom_img_alloc(&img, AOM_IMG_FMT_I420, width, height, 1);
  libyuv::ARGBToI420(staging.data(), stagingStride, img.planes[0], img.stride[0], img.planes[1], img.stride[1], img.planes[2], img.stride[2], width, height);

  std::vector<std::vector<uint8_t>> packets;

  { // encode a frame
    aom_codec_cx_pkt_t const* pkt;
    aom_codec_iter_t iter = nullptr;
    aom_codec_err_t const res = aom_codec_encode(&codec, &img, 0, 1, AOM_EFLAG_FORCE_KF);
    if (res != AOM_CODEC_OK) {
      log.fatal("failed to encode a key frame");
    }
    while ((pkt = aom_codec_get_cx_data(&codec, &iter)) != nullptr) {
      if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
        auto& frame = pkt->data.frame;
        auto const beg = reinterpret_cast<uint8_t*>(frame.buf);
        packets.emplace_back(std::vector<uint8_t>(beg, beg + frame.sz));
      }
      break;
    }
  }
  { // flushing
    bool cont = true;
    while(cont) {
      aom_codec_iter_t iter = nullptr;
      aom_codec_cx_pkt_t const* pkt = nullptr;
      const aom_codec_err_t res = aom_codec_encode(&codec, nullptr, -1, 1, 0);
      if (res != AOM_CODEC_OK) {
        log.fatal("failed to flushing encoder");
      }
      bool gotPkt = false;
      while ((pkt = aom_codec_get_cx_data(&codec, &iter)) != nullptr) {
        gotPkt = true;
        if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
          auto& frame = pkt->data.frame;
          auto const beg = reinterpret_cast<uint8_t*>(frame.buf);
          packets.emplace_back(std::vector<uint8_t>(beg, beg + frame.sz));
        }
      }
      cont &= gotPkt;
    }
  }
  aom_img_free(&img);
  if (aom_codec_destroy(&codec) != AOM_CODEC_OK) {
    log.error("Failed to destroy codec.");
    return -1;
  }
  if(packets.empty()) {
    log.error("no packats to out.");
    return -1;
  }
  {
    AVIFBuilder builder(width, height);

    std::shared_ptr<avif::av1::Parser::Result> result = avif::av1::Parser(log, packets[0]).parse();
    if (!result->ok()) {
      log.error(result->error());
      return -1;
    }
    avif::av1::SequenceHeader seq{};
    std::vector<uint8_t> configOBU;
    std::vector<uint8_t> mdat;
    for(avif::av1::Parser::Result::Packet const& packet : result->packets()) {
      switch (packet.type()) {
        case avif::av1::Header::Type::TemporalDelimiter:
        case avif::av1::Header::Type::Padding:
        case avif::av1::Header::Type::Reserved:
          break;
        case avif::av1::Header::Type::SequenceHeader:
          seq = std::get<avif::av1::SequenceHeader>(packet.content());
          configOBU.insert(std::end(configOBU), std::next(std::begin(result->buffer()), packet.beg()), std::next(result->buffer().begin(), packet.end()));
          mdat.insert(std::end(mdat), std::next(std::begin(result->buffer()), packet.beg()), std::next(std::begin(result->buffer()), packet.end()));
          break;
        case avif::av1::Header::Type::Frame:
        default: {
          mdat.insert(std::end(mdat), std::next(std::begin(result->buffer()), packet.beg()), std::next(std::begin(result->buffer()), packet.end()));
          break;
        }
      }
    }
    builder.setPrimaryFrame(AVIFBuilder::Frame(seq, std::move(configOBU), mdat));
    avif::FileBox fileBox = builder.build();
    {
      avif::util::StreamWriter pass1;
      avif::Writer(log, pass1).write(fileBox);
    }
    for (size_t i = 0; i < fileBox.metaBox.itemLocationBox.items.size(); ++i) {
      size_t const offset = fileBox.mediaDataBoxes.at(i).offset;
      fileBox.metaBox.itemLocationBox.items.at(i).baseOffset = offset;
    }
    avif::util::StreamWriter out;
    avif::Writer(log, out).write(fileBox);
    std::vector<uint8_t> data = out.buffer();
    std::copy(std::begin(mdat), std::end(mdat), std::next(std::begin(data), fileBox.mediaDataBoxes.at(0).offset));
    std::optional<std::string> writeResult = avif::util::writeFile(outputFilename, data);
    if (writeResult.has_value()) {
      log.error(writeResult.value());
      return -1;
    }
  }
  return 0;
}
