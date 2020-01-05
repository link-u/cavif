#include <iostream>

#include "../external/clipp/include/clipp.h"
#include "Image.hpp"
#include "PNGReader.hpp"
#include <aom/aom_encoder.h>
#include <aom/aom_codec.h>
#include <aom/aomcx.h>
#include <libyuv.h>

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
  std::string inputFilename = {};
  std::string outputFilename = {};
  aom_codec_ctx_t codec = {};
  aom_codec_enc_cfg cfg = {};
  aom_codec_iface_t* av1codec = aom_codec_av1_cx();
  if(!av1codec){
    std::cerr << ("Failed to get AV1 encoder.");
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
    std::cerr << "Please give png or bmp file for input" << std::endl;
    return -3;
  }
  uint32_t const width = srcImage.width();
  uint32_t const height = srcImage.height();

  // converting RGB(A) -> I444
  // FIXME(ledyba-z): how about alpha channels?
  std::vector<uint8_t> staging;
  uint32_t statingStride = 0;
  uint32_t statingBytesPerPiexl = 4;
  switch(srcImage.type()) {
    case img::Image::Type::RGB:
      statingStride = statingBytesPerPiexl * width;
      staging.resize(height * statingStride);
      libyuv::RGB24ToARGB(srcImage.data().data(), srcImage.stride(), staging.data(), statingStride, width, height);
      break;
    case img::Image::Type::RGBA:
      statingStride = statingBytesPerPiexl * width;
      staging.resize(height * statingStride);
      libyuv::RGBAToARGB(srcImage.data().data(), srcImage.stride(), staging.data(), statingStride, width, height);
  }

  // initialize encoder
  cfg.g_w = width;
  cfg.g_h = height;
  if(AOM_CODEC_OK != aom_codec_enc_init(&codec, av1codec, &cfg, 0)) {
    std::cerr << "Failed to initialize encoder." << std::endl;
    return -2;
  }

  aom_image_t img;
  // FIXME: read validate_img() function.
  // https://github.com/link-u/libaom/blob/d0b3d306aebb5ef6cc89f49f56dd7adaee41f696/av1/av1_cx_iface.c#L532
  if(cfg.monochrome) {
    aom_img_alloc(&img, AOM_IMG_FMT_I420, width, height, 1);
    libyuv::ARGBToI420(staging.data(), statingStride, img.planes[0], img.stride[0], img.planes[1], img.stride[1], img.planes[2], img.stride[2], width, height);
  } else {
    aom_img_alloc(&img, AOM_IMG_FMT_I444, width, height, 1);
    libyuv::ARGBToI444(staging.data(), statingStride, img.planes[0], img.stride[0], img.planes[1], img.stride[1], img.planes[2], img.stride[2], width, height);
  }

  { // encode a frame
    aom_codec_cx_pkt_t const* pkt;
    aom_codec_iter_t iter = nullptr;
    aom_codec_err_t const res = aom_codec_encode(&codec, &img, 0, 1, AOM_EFLAG_FORCE_KF);
    if (res != AOM_CODEC_OK) {
      throw std::runtime_error("failed to encode a key frame");
    }
    while ((pkt = aom_codec_get_cx_data(&codec, &iter)) != nullptr) {
      if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
        auto& frame = pkt->data.frame;
        // TODO: parse OBU and store data
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
        throw std::runtime_error("failed to encode");
      }
      bool gotPkt = false;
      while ((pkt = aom_codec_get_cx_data(&codec, &iter)) != nullptr) {
        gotPkt = true;
        if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
          auto& frame = pkt->data.frame;
          // TODO: parse OBU and store data
        }
      }
      cont &= gotPkt;
    }
  }
  aom_img_free(&img);
  if (aom_codec_destroy(&codec) != AOM_CODEC_OK) {
    std::cerr << "Failed to destroy codec.";
    return -1;
  }
  return 0;
}
