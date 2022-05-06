//
// Created by psi on 2020/01/18.
//

#include <stdexcept>
#include <fmt/format.h>

#include <avif/img/Image.hpp>
#include <avif/img/Conversion.hpp>
#include <avif/img/color/Constants.hpp>
#include <avif/img/color/Matrix.hpp>

using MatrixCoefficients = avif::img::color::MatrixCoefficients;

namespace detail {

template <typename Converter, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange>
void convertImage(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.monochrome) {
    avif::img::FromRGB<Converter, rgbBits, yuvBits, fromMonoRGB, isFullRange>::toI400(
        src,
        dst.planes[0], dst.stride[0]);
  } else {
    switch (dst.fmt) {
      case AOM_IMG_FMT_I420:
      case AOM_IMG_FMT_I42016:
        avif::img::FromRGB<Converter, rgbBits, yuvBits, fromMonoRGB, isFullRange>::toI420(
            src,
            dst.planes[0], dst.stride[0],
            dst.planes[1], dst.stride[1],
            dst.planes[2], dst.stride[2]);
        break;
      case AOM_IMG_FMT_I422:
      case AOM_IMG_FMT_I42216:
        avif::img::FromRGB<Converter, rgbBits, yuvBits, fromMonoRGB, isFullRange>::toI422(
            src,
            dst.planes[0], dst.stride[0],
            dst.planes[1], dst.stride[1],
            dst.planes[2], dst.stride[2]);
        break;
      case AOM_IMG_FMT_I444:
      case AOM_IMG_FMT_I44416:
        avif::img::FromRGB<Converter, rgbBits, yuvBits, fromMonoRGB, isFullRange>::toI444(
            src,
            dst.planes[0], dst.stride[0],
            dst.planes[1], dst.stride[1],
            dst.planes[2], dst.stride[2]);
        break;
      default:
        throw std::invalid_argument(fmt::format("Unsupported image format: {:08x}", dst.fmt));
    }
  }
}

template <typename Converter, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB>
void convertImage(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.range == AOM_CR_FULL_RANGE) {
    convertImage<Converter, rgbBits, yuvBits, fromMonoRGB, true>(src, dst);
  } else {
    convertImage<Converter, rgbBits, yuvBits, fromMonoRGB, false>(src, dst);
  }
}

template <typename Converter, uint8_t rgbBits, uint8_t yuvBits>
void convertImage(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(src.isMonochrome()) {
    convertImage<Converter, rgbBits, yuvBits, true>(src, dst);
  } else {
    convertImage<Converter, rgbBits, yuvBits, false>(src, dst);
  }
}

template <typename Converter, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
void convertAlpha(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if (dst.monochrome) {
    avif::img::FromAlpha<Converter, rgbBits, yuvBits, isFullRange>::toI400(
        src,
        dst.planes[0], dst.stride[0]);
  } else {
    throw std::invalid_argument("Alpha image should be monochrome. Please add --monochrome option.");
  }
}

template <typename Converter, uint8_t rgbBits, uint8_t yuvBits>
void convertAlpha(avif::img::Image<rgbBits>& src, aom_image& dst) {
  convertAlpha<Converter, rgbBits, yuvBits, true>(src, dst);
}

}

template <typename Converter, Config::EncodeTarget target, uint8_t rgbBits, uint8_t yuvBits>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (target) {
    case Config::EncodeTarget::Image:
      detail::convertImage<Converter, rgbBits, yuvBits>(src, dst);
      break;
    case Config::EncodeTarget::Alpha:
      detail::convertAlpha<Converter, rgbBits, yuvBits>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported EncodeTarget: {}", target));
  }
}

template <typename Converter, Config::EncodeTarget target, size_t rgbBits>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (dst.bit_depth) {
    case 8:
      convert<Converter, target, rgbBits, 8>(src, dst);
      break;
    case 10:
      convert<Converter, target, rgbBits, 10>(src, dst);
      break;
    case 12:
      convert<Converter, target, rgbBits, 12>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported YUV bit-depth: {}", dst.bit_depth));
  }
}

template <typename Converter, size_t rgbBits>
void convert(Config& config, avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (config.encodeTarget) {
    case Config::EncodeTarget::Image:
      convert<Converter, Config::EncodeTarget::Image, rgbBits>(src, dst);
      break;
    case Config::EncodeTarget::Alpha:
      convert<Converter, Config::EncodeTarget::Alpha, rgbBits>(src, dst);
      break;
    default:
      assert(false && "[BUG] Unknown encoder target.");
  }
}

template <size_t rgbBits>
void convert(Config& config, avif::img::Image<rgbBits>& src, aom_image& dst) {
  aom_img_fmt_t const pixFmt =
      config.codec.g_bit_depth == 8 ?
        config.pixFmt :
        static_cast<aom_img_fmt_t>(config.pixFmt | static_cast<unsigned int>(AOM_IMG_FMT_HIGHBITDEPTH));
  aom_img_alloc(&dst, pixFmt, src.width(), src.height(), 1);
  dst.range = config.fullColorRange ? AOM_CR_FULL_RANGE : AOM_CR_STUDIO_RANGE;
  dst.monochrome = config.codec.monochrome ? 1 : 0;
  dst.bit_depth = config.codec.g_bit_depth;

  switch (static_cast<MatrixCoefficients>(src.colorProfile().cicp.value_or(avif::ColourInformationBox::CICP()).matrixCoefficients)) {
    case MatrixCoefficients::MC_IDENTITY: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_IDENTITY>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_709: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_BT_709>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_UNSPECIFIED: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_UNSPECIFIED>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_RESERVED_3: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_RESERVED_3>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_FCC: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_FCC>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_470_B_G: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_BT_470_B_G>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_NSTC: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_NSTC>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_SMPTE_240: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_SMPTE_240>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_SMPTE_YCGCO: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_SMPTE_YCGCO>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_2020_NCL: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_BT_2020_NCL>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_2020_CL:{
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_BT_2020_CL>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_SMPTE_2085:{
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_SMPTE_2085>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_CHROMAT_NCL:{
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_CHROMAT_NCL>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_CHROMAT_CL:{
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_CHROMAT_CL>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_2100_ICTCP: {
      using ConvereterType = avif::img::color::ColorConverter<MatrixCoefficients::MC_BT_2100_ICTCP>;
      convert<ConvereterType, rgbBits>(config, src, dst);
      break;
    }
    default:
      assert(false && "Unknown matrix coefficients");
  }
}
