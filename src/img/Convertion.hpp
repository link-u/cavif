//
// Created by psi on 2020/01/18.
//

#include <stdexcept>
#include <fmt/format.h>
#include "avif/img/Image.hpp"
#include "avif/img/Conversion.hpp"

template <uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.monochrome) {
    avif::img::FromRGB<rgbBits, yuvBits, isFullRange>().toI400(src,
                                                               dst.planes[0], dst.stride[0]);
  } else {
    switch (dst.fmt) {
      case AOM_IMG_FMT_I420:
      case AOM_IMG_FMT_I42016:
        avif::img::FromRGB<rgbBits, yuvBits, isFullRange>().toI420(src,
                                                                   dst.planes[0], dst.stride[0],
                                                                   dst.planes[1], dst.stride[1],
                                                                   dst.planes[2], dst.stride[2]);
        break;
      case AOM_IMG_FMT_I422:
      case AOM_IMG_FMT_I42216:
        avif::img::FromRGB<rgbBits, yuvBits, isFullRange>().toI422(src,
                                                                   dst.planes[0], dst.stride[0],
                                                                   dst.planes[1], dst.stride[1],
                                                                   dst.planes[2], dst.stride[2]);
        break;
      case AOM_IMG_FMT_I444:
      case AOM_IMG_FMT_I44416:
        avif::img::FromRGB<rgbBits, yuvBits, isFullRange>().toI444(src,
                                                                   dst.planes[0], dst.stride[0],
                                                                   dst.planes[1], dst.stride[1],
                                                                   dst.planes[2], dst.stride[2]);
        break;
      default:
        throw std::invalid_argument(fmt::format("Unsupported image format: {:08x}", dst.fmt));
    }
  }
}

template <uint8_t rgbBits, uint8_t yuvBits>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch(dst.range) {
    case AOM_CR_STUDIO_RANGE:
      convert<rgbBits, yuvBits, false>(src, dst);
      break;
    case AOM_CR_FULL_RANGE:
      convert<rgbBits, yuvBits, true>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported color range type: {}", dst.range));
  }
}

template <size_t rgbBits>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst, int const yuvBits) {
  switch (yuvBits) {
    case 8:
      convert<rgbBits, 8>(src, dst);
      break;
    case 10:
      convert<rgbBits, 10>(src, dst);
      break;
    case 12:
      convert<rgbBits, 12>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported YUV bit-depth: {}", dst.bit_depth));
  }
}
