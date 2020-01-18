#include <iostream>

#include "img/Image.hpp"
#include "img/PNGReader.hpp"
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
#include <thread>

#include "Configurator.hpp"
#include "img/ImageConverter.hpp"

namespace {

// FIXME(ledyba-z): remove this function when the C++20 comes.
bool endsWidh(std::string const& target, std::string const& suffix) {
  if(target.size() < suffix.size()) {
    return false;
  }
  return target.substr(target.size()-suffix.size()) == suffix;
}



}

int main(int argc, char** argv) {
  avif::util::FileLogger log(stdout, stderr, avif::util::Logger::Level::DEBUG);

  aom_codec_iface_t* av1codec = aom_codec_av1_cx();
  if(!av1codec) {
    log.fatal("failed to get AV1 encoder.");
  }

  Configurator config;
  aom_codec_enc_config_default(av1codec, &config.encoderConfig, 0);
  {
    int const result = config.parse(argc, argv);
    if(result != 0) {
      return result;
    }
  }

  // decoding input image
  Image srcImage;
  if(endsWidh(config.input, ".png")) {
    srcImage = PNGReader(config.input).read();
  } else if(endsWidh(config.input, ".bmp")) {

  } else {
    log.fatal("please give png or bmp file for input");
  }
  uint32_t const width = srcImage.width();
  uint32_t const height = srcImage.height();

  aom_image_t img;
  aom_img_alloc(&img, config.outPixFmt, width, height, 1);
  ImageConverter(srcImage, img).convert();

  // initialize encoder
  config.encoderConfig.g_w = width;
  config.encoderConfig.g_h = height;
  // Generate just one frame.
  config.encoderConfig.g_limit = 1;
  config.encoderConfig.g_pass = AOM_RC_ONE_PASS;
  // FIXME(ledyba-z):
  // > A value of 0 implies all frames will be keyframes.
  // However, when it is set to 0, assertion always fails:
  // cavif/external/libaom/av1/encoder/gop_structure.c:92:
  // construct_multi_layer_gf_structure: Assertion `gf_interval >= 1' failed.
  config.encoderConfig.kf_max_dist = 1;
  // One frame takes 1 second.
  config.encoderConfig.g_timebase.den = 1;
  config.encoderConfig.g_timebase.num = 1;
  //
  config.encoderConfig.rc_target_bitrate = 0;

  aom_codec_ctx_t codec{};
  if(AOM_CODEC_OK != aom_codec_enc_init(&codec, av1codec, &config.encoderConfig, 0)) {
    log.fatal("Failed to initialize encoder.");
  }

  config.modify(&codec);

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
    std::optional<avif::av1::SequenceHeader> seq{};
    std::vector<uint8_t> configOBUs;
    std::vector<uint8_t> mdat;
    for(avif::av1::Parser::Result::Packet const& packet : result->packets()) {
      switch (packet.type()) {
        case avif::av1::Header::Type::TemporalDelimiter:
        case avif::av1::Header::Type::Padding:
        case avif::av1::Header::Type::Reserved:
          break;
        case avif::av1::Header::Type::SequenceHeader:
          seq = std::get<avif::av1::SequenceHeader>(packet.content());
          configOBUs.insert(std::end(configOBUs), std::next(std::begin(result->buffer()), packet.beg()), std::next(result->buffer().begin(), packet.end()));
          mdat.insert(std::end(mdat), std::next(std::begin(result->buffer()), packet.beg()), std::next(std::begin(result->buffer()), packet.end()));
          break;
        case avif::av1::Header::Type::Frame:
        default: {
          mdat.insert(std::end(mdat), std::next(std::begin(result->buffer()), packet.beg()), std::next(std::begin(result->buffer()), packet.end()));
          break;
        }
      }
    }
    if (!seq.has_value()) {
      throw std::logic_error("No sequence header OBU.");
    }
    builder.setPrimaryFrame(AVIFBuilder::Frame(seq.value(), std::move(configOBUs), mdat));
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
    std::optional<std::string> writeResult = avif::util::writeFile(config.output, data);
    if (writeResult.has_value()) {
      log.error(writeResult.value());
      return -1;
    }
  }
  return 0;
}
