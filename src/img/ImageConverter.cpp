//
// Created by psi on 2020/01/18.
//

#include <libyuv.h>
#include <cassert>
#include <stdexcept>
#include "ImageConverter.hpp"
#include "../prism/Image.hpp"
#include "../prism/BT2020.hpp"

ImageConverter::ImageConverter(prism::Image& src, aom_image& dst)
:width_(src.width())
,height_(src.height())
,src_(src)
,dst_(dst)
{
}

void ImageConverter::convert(uint8_t bitsPerComponent) {
  prism::Image& src = src_;
  if(bitsPerComponent == 8) {
    std::vector<uint8_t> staging{};
    size_t stagingStride = 0;
    switch (src.type()) {
      case prism::Image::Type::RGB: {
        std::vector<uint8_t> abgr{};
        size_t const abgrStride = 4 * width_;
        abgr.resize(abgrStride * height_);
        // BGR24ToABGRが無かった
        if(0 != libyuv::RGB24ToARGB(src.data<uint8_t>(), src.stride(), abgr.data(), abgrStride, width_, height_)){
          throw std::runtime_error("Failed to convert from RGB to YUV, on calling RGB24ToARGB");
        }
        // BGR24ToARGBが無かった
        stagingStride = 4 * width_;
        staging.resize(stagingStride * height_);
        if(libyuv::ABGRToARGB(abgr.data(), abgrStride, staging.data(), stagingStride, width_, height_)){
          throw std::runtime_error("Failed to convert from RGB to YUV, on calling ABGRToARGB");
        }
        convertFromBGRA8888(staging, stagingStride);
        break;
      }
      case prism::Image::Type::RGBA:
        stagingStride = src.stride();
        staging.resize(stagingStride * height_);
        if( 0 != libyuv::ABGRToARGB(src.data<uint8_t>(), src.stride(), staging.data(), stagingStride, width_, height_)){
          throw std::runtime_error("Failed to convert from RGB to YUV, on calling ABGRToARGB");
        }
        convertFromBGRA8888(staging, stagingStride);
        break;
    }
  } else {
    switch (dst_.fmt) {
      case AOM_IMG_FMT_I42016:
        prism::BT2020(bitsPerComponent, 2.2f).convertI420(src,
                                                          reinterpret_cast<uint16_t*>(dst_.planes[0]), dst_.stride[0],
                                                          reinterpret_cast<uint16_t*>(dst_.planes[1]), dst_.stride[1],
                                                          reinterpret_cast<uint16_t*>(dst_.planes[2]), dst_.stride[2]);
        break;
      case AOM_IMG_FMT_I42216:
        prism::BT2020(bitsPerComponent, 2.2f).convertI422(src,
                                                          reinterpret_cast<uint16_t*>(dst_.planes[0]), dst_.stride[0],
                                                          reinterpret_cast<uint16_t*>(dst_.planes[1]), dst_.stride[1],
                                                          reinterpret_cast<uint16_t*>(dst_.planes[2]), dst_.stride[2]);
        break;
      case AOM_IMG_FMT_I44416:
        prism::BT2020(bitsPerComponent, 2.2f).convertI444(src,
                                                          reinterpret_cast<uint16_t*>(dst_.planes[0]), dst_.stride[0],
                                                          reinterpret_cast<uint16_t*>(dst_.planes[1]), dst_.stride[1],
                                                          reinterpret_cast<uint16_t*>(dst_.planes[2]), dst_.stride[2]);
        break;
    }
  }
}


void ImageConverter::convertFromBGRA8888(std::vector<uint8_t> const& bgra, size_t const stride) {
  bool ok = false;
  switch (dst_.fmt) {
    case AOM_IMG_FMT_I420:
      ok = 0 == libyuv::ARGBToI420(
          bgra.data(), stride,
          dst_.planes[0], dst_.stride[0],
          dst_.planes[1], dst_.stride[1],
          dst_.planes[2], dst_.stride[2],
          src_.width(), src_.height());
      break;
    case AOM_IMG_FMT_I422:
      ok = 0 == libyuv::ARGBToI422(
          bgra.data(), stride,
          dst_.planes[0], dst_.stride[0],
          dst_.planes[1], dst_.stride[1],
          dst_.planes[2], dst_.stride[2],
          src_.width(), src_.height());
      break;
    case AOM_IMG_FMT_I444:
      ok = 0 == libyuv::ARGBToI444(
          bgra.data(), stride,
          dst_.planes[0], dst_.stride[0],
          dst_.planes[1], dst_.stride[1],
          dst_.planes[2], dst_.stride[2],
          src_.width(), src_.height());
      break;
  }
  if(!ok) {
    throw std::runtime_error("Failed to convert from BGRA to YUV, on calling ABGRToI4xx");
  }
}
