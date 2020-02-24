//
// Created by psi on 2020/01/18.
//

#include <stdexcept>
#include <fmt/format.h>
#include "avif/img/Image.hpp"
#include "avif/img/Conversion.hpp"

namespace detail {

template <uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
void convertImage(avif::img::Image<rgbBits>& src, aom_image& dst) {
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

template <uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
void convertAlpha(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if (dst.monochrome) {
    avif::img::FromAlpha<rgbBits, yuvBits, isFullRange>().toI400(src,
                                                                 dst.planes[0], dst.stride[0]);
  } else {
    throw std::invalid_argument("Alpha image should be monochrome");
  }
}

}

template <Config::EncodeTarget target, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (target) {
    case Config::EncodeTarget::Image:
      detail::convertImage<rgbBits, yuvBits, isFullRange>(src, dst);
      break;
    case Config::EncodeTarget::Alpha:
      detail::convertAlpha<rgbBits, yuvBits, isFullRange>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported EncodeTarget: {}", target));
  }
}

template <Config::EncodeTarget target, uint8_t rgbBits, uint8_t yuvBits>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch(dst.range) {
    case AOM_CR_STUDIO_RANGE:
      convert<target, rgbBits, yuvBits, false>(src, dst);
      break;
    case AOM_CR_FULL_RANGE:
      convert<target, rgbBits, yuvBits, true>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported color range type: {}", dst.range));
  }
}

template <Config::EncodeTarget target, size_t rgbBits>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (dst.bit_depth) {
    case 8:
      convert<target, rgbBits, 8>(src, dst);
      break;
    case 10:
      convert<target, rgbBits, 10>(src, dst);
      break;
    case 12:
      convert<target, rgbBits, 12>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported YUV bit-depth: {}", dst.bit_depth));
  }
}

template <size_t rgbBits>
void convert(Config& config, avif::img::Image<rgbBits>& src, aom_image& dst) {
  aom_img_fmt_t const pixFmt = config.codec.g_bit_depth ?
        config.pixFmt :
        static_cast<aom_img_fmt_t>(config.pixFmt | static_cast<unsigned int>(AOM_IMG_FMT_HIGHBITDEPTH));
  aom_img_alloc(&dst, pixFmt, src.width(), src.height(), 1);
  dst.range = config.fullColorRange ? AOM_CR_FULL_RANGE : AOM_CR_STUDIO_RANGE;
  dst.monochrome = config.codec.monochrome ? 1 : 0;
  dst.bit_depth = config.codec.g_bit_depth;
  switch (config.encodeTarget) {
    case Config::EncodeTarget::Image:
      convert<Config::EncodeTarget::Image, rgbBits>(src, dst);
      break;
    case Config::EncodeTarget::Alpha:
      convert<Config::EncodeTarget::Alpha, rgbBits>(src, dst);
      break;
    default:
      assert(false && "[BUG] Unkown encoder target.");
  }
}
