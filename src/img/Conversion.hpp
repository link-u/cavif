//
// Created by psi on 2020/01/18.
//

#include <stdexcept>
#include <fmt/format.h>
#include "avif/img/Image.hpp"
#include "avif/img/Conversion.hpp"

using MatrixCoefficients = avif::img::MatrixCoefficients;

namespace detail {

template <typename ConverterImpl, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange>
void convertImage(ConverterImpl const& converter, avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.monochrome) {
    avif::img::FromRGB<ConverterImpl, rgbBits, yuvBits, fromMonoRGB, isFullRange>::toI400(
        converter,
        src,
        dst.planes[0], dst.stride[0]);
  } else {
    switch (dst.fmt) {
      case AOM_IMG_FMT_I420:
      case AOM_IMG_FMT_I42016:
        avif::img::FromRGB<ConverterImpl, rgbBits, yuvBits, fromMonoRGB, isFullRange>::toI420(
            converter,
            src,
            dst.planes[0], dst.stride[0],
            dst.planes[1], dst.stride[1],
            dst.planes[2], dst.stride[2]);
        break;
      case AOM_IMG_FMT_I422:
      case AOM_IMG_FMT_I42216:
        avif::img::FromRGB<ConverterImpl, rgbBits, yuvBits, fromMonoRGB, isFullRange>::toI422(
            converter,
            src,
            dst.planes[0], dst.stride[0],
            dst.planes[1], dst.stride[1],
            dst.planes[2], dst.stride[2]);
        break;
      case AOM_IMG_FMT_I444:
      case AOM_IMG_FMT_I44416:
        avif::img::FromRGB<ConverterImpl, rgbBits, yuvBits, fromMonoRGB, isFullRange>::toI444(
            converter,
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

template <typename ConverterImpl, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB>
void convertImage(ConverterImpl const& converter, avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.range == AOM_CR_FULL_RANGE) {
    convertImage<ConverterImpl, rgbBits, yuvBits, fromMonoRGB, true>(converter, src, dst);
  } else {
    convertImage<ConverterImpl, rgbBits, yuvBits, fromMonoRGB, false>(converter, src, dst);
  }
}

template <typename ConverterImpl, uint8_t rgbBits, uint8_t yuvBits>
void convertImage(ConverterImpl const& converter, avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(src.isMonochrome()) {
    convertImage<ConverterImpl, rgbBits, yuvBits, true>(converter, src, dst);
  } else {
    convertImage<ConverterImpl, rgbBits, yuvBits, false>(converter, src, dst);
  }
}

template <typename ConverterImpl, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
void convertAlpha(ConverterImpl const& converter, avif::img::Image<rgbBits>& src, aom_image& dst) {
  if (dst.monochrome) {
    avif::img::FromAlpha<ConverterImpl, rgbBits, yuvBits, isFullRange>::toI400(
        converter,
        src,
        dst.planes[0], dst.stride[0]);
  } else {
    throw std::invalid_argument("Alpha image should be monochrome. Please add --monochrome option.");
  }
}

template <typename ConverterImpl, uint8_t rgbBits, uint8_t yuvBits>
void convertAlpha(ConverterImpl const& converter, avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.range == AOM_CR_FULL_RANGE) {
    convertAlpha<ConverterImpl, rgbBits, yuvBits, true>(converter, src, dst);
  } else {
    convertAlpha<ConverterImpl, rgbBits, yuvBits, false>(converter, src, dst);
  }
}

}

template <typename ConverterImpl, Config::EncodeTarget target, uint8_t rgbBits, uint8_t yuvBits>
void convert(ConverterImpl const& converter, avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (target) {
    case Config::EncodeTarget::Image:
      detail::convertImage<ConverterImpl, rgbBits, yuvBits>(converter, src, dst);
      break;
    case Config::EncodeTarget::Alpha:
      detail::convertAlpha<ConverterImpl, rgbBits, yuvBits>(converter, src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported EncodeTarget: {}", target));
  }
}

template <typename ConverterImpl, Config::EncodeTarget target, size_t rgbBits>
void convert(ConverterImpl const& converter, avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (dst.bit_depth) {
    case 8:
      convert<ConverterImpl, target, rgbBits, 8>(converter, src, dst);
      break;
    case 10:
      convert<ConverterImpl, target, rgbBits, 10>(converter, src, dst);
      break;
    case 12:
      convert<ConverterImpl, target, rgbBits, 12>(converter, src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported YUV bit-depth: {}", dst.bit_depth));
  }
}

template <typename ConverterImpl, size_t rgbBits>
void convert(Config& config, ConverterImpl const& converter, avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (config.encodeTarget) {
    case Config::EncodeTarget::Image:
      convert<ConverterImpl, Config::EncodeTarget::Image, rgbBits>(converter, src, dst);
      break;
    case Config::EncodeTarget::Alpha:
      convert<ConverterImpl, Config::EncodeTarget::Alpha, rgbBits>(converter, src, dst);
      break;
    default:
      assert(false && "[BUG] Unkown encoder target.");
  }
}

template <size_t rgbBits>
void convert(Config& config, avif::img::Image<rgbBits>& src, aom_image& dst) {
  aom_img_fmt_t const pixFmt = config.codec.g_bit_depth == 8 ?
        config.pixFmt :
        static_cast<aom_img_fmt_t>(config.pixFmt | static_cast<unsigned int>(AOM_IMG_FMT_HIGHBITDEPTH));
  aom_img_alloc(&dst, pixFmt, src.width(), src.height(), 1);
  dst.range = config.fullColorRange ? AOM_CR_FULL_RANGE : AOM_CR_STUDIO_RANGE;
  dst.monochrome = config.codec.monochrome ? 1 : 0;
  dst.bit_depth = config.codec.g_bit_depth;
  if(std::holds_alternative<avif::img::ICCProfile>(src.colorProfile())) {
    avif::img::ICCProfile const& profile = std::get<avif::img::ICCProfile>(src.colorProfile());
    auto icc = profile.calcColorCoefficients();
      convert<decltype(icc), rgbBits>(config, icc, src, dst);
    return;
  }
  namespace converters = avif::img::converters;
  auto matrix = static_cast<MatrixCoefficients>(config.matrixCoefficients);
  switch (matrix) {
    case MatrixCoefficients::MC_IDENTITY: {
      constexpr auto converter = converters::Identity;
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_UNSPECIFIED:
    case MatrixCoefficients::MC_BT_709: {
      constexpr auto converter = converters::BT_709;
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_FCC: {
      constexpr auto converter = converters::FCC;
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_470_B_G: {
      constexpr auto converter = converters::BT_470_B_G;
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_601: {
      constexpr auto converter = converters::BT_601;
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_SMPTE_240: {
      constexpr auto converter = converters::SMPTE_240;
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_SMPTE_YCGCO: {
      constexpr auto converter = converters::Unimplementd(MatrixCoefficients::MC_SMPTE_YCGCO);
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_2020_NCL: {
      auto converter = converters::Unimplementd(matrix);
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_BT_2020_CL:{
      auto converter = converters::Unimplementd(matrix);
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_SMPTE_2085:{
      auto converter = converters::Unimplementd(matrix);
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_CHROMAT_NCL:{
      auto converter = converters::Unimplementd(matrix);
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_CHROMAT_CL:{
      auto converter = converters::Unimplementd(matrix);
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    case MatrixCoefficients::MC_ICTCP:{
      auto converter = converters::Unimplementd(matrix);
      convert<decltype(converter), rgbBits>(config, converter, src, dst);
      break;
    }
    default:
      assert(false && "Unknown matrix coefficients");
  }
}
