//
// Created by psi on 2020/01/18.
//

#include <cassert>
#include <stdexcept>
#include <fmt/format.h>
#include "Convert.hpp"
#include "../prism/Image.hpp"
#include "../prism/Conversion.hpp"

template <uint8_t rgbBits, uint8_t yuvBits>
void convert(prism::Image& src, aom_image& dst) {
  switch (dst.fmt) {
    case AOM_IMG_FMT_I420:
    case AOM_IMG_FMT_I42016:
      prism::FromRGB<rgbBits, yuvBits>().toI420(src,
                                    dst.planes[0], dst.stride[0],
                                    dst.planes[1], dst.stride[1],
                                    dst.planes[2], dst.stride[2]);
      break;
    case AOM_IMG_FMT_I422:
    case AOM_IMG_FMT_I42216:
      prism::FromRGB<rgbBits, yuvBits>().toI422(src,
                                        dst.planes[0], dst.stride[0],
                                        dst.planes[1], dst.stride[1],
                                        dst.planes[2], dst.stride[2]);
      break;
    case AOM_IMG_FMT_I444:
    case AOM_IMG_FMT_I44416:
      prism::FromRGB<rgbBits, yuvBits>().toI444(src,
                                        dst.planes[0], dst.stride[0],
                                        dst.planes[1], dst.stride[1],
                                        dst.planes[2], dst.stride[2]);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported image format: {:08x}", dst.fmt));
  }
}


void convert(prism::Image& src, aom_image& dst) {
  if (src.bitsPerComponent() == 8) {
    switch (dst.bit_depth) {
      case 8:
        convert<8, 8>(src, dst);
        break;
      case 10:
        convert<8, 10>(src, dst);
        break;
      case 12:
        convert<8, 12>(src, dst);
        break;
      default:
        throw std::invalid_argument(fmt::format("Unsupported YUV bit-depth: {}", dst.bit_depth));
    }
  } else if (src.bitsPerComponent() == 16) {
    switch (dst.bit_depth) {
      case 8:
        convert<16, 8>(src, dst);
        break;
      case 10:
        convert<16, 10>(src, dst);
        break;
      case 12:
        convert<16, 12>(src, dst);
        break;
      default:
        throw std::invalid_argument(fmt::format("Unsupported YUV bit-depth: {}", dst.bit_depth));
    }
  } else {
      throw std::invalid_argument(fmt::format("Unsupported RGB bit-depth: {}", src.bitsPerComponent()));
  }
}
