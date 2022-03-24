//
// Created by psi on 2020/01/05.
//

#include "Reader.hpp"
#include <filesystem>
#include <system_error>
#include "fmt/format.h"

namespace img::png {

Reader::Reader(avif::util::FileLogger& log, FILE* const file, png_structp png, png_infop info)
:log_(log)
,file_(file)
,png_(png)
,info_(info)
{
}

char const* Reader::version() {
  return PNG_LIBPNG_VER_STRING;
}

Reader Reader::create(avif::util::FileLogger& log, std::string const& filename) {
  // See http://www.libpng.org/pub/png/libpng-manual.txt
  FILE* const file = fopen(filename.c_str(), "rb");
  if(!file) {
    throw std::filesystem::filesystem_error("failed to open", filename, std::make_error_code(static_cast<std::errc>(errno)));
  }

  { // Check header
    uint8_t header[8] = {};
    if (fread(header, 1, 8, file) != 8) {
      throw std::filesystem::filesystem_error("failed to read header", filename, std::make_error_code(static_cast<std::errc>(errno)));
    }
    if (png_sig_cmp(header, 0, 8)) {
      throw std::runtime_error(fmt::format("{} is not a valid PNG file.", filename));
    }
  }

  // Create reader and retrieve info
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if(!png) {
    throw std::runtime_error("failed to create png reader");
  }

  png_infop info = png_create_info_struct(png);
  if(!info) {
    throw std::runtime_error("failed to create png info structure");
  }

  png_init_io(png, file);
  png_set_sig_bytes(png, 8);

  return Reader(log, file, png, info);
}

Reader::~Reader() noexcept {
  // Clean up reader
  png_destroy_read_struct(&png_, &info_, nullptr);
  // Close file
  fclose(file_);
}

Reader::Result Reader::read() {
  png_read_info(png_, info_);

  uint32_t const width = png_get_image_width(png_, info_);
  uint32_t const height = png_get_image_height(png_, info_);
  png_byte const colorType = png_get_color_type(png_, info_);
  png_byte const bitDepth  = png_get_bit_depth(png_, info_);

  if(colorType == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_);
  }

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  // http://www.libpng.org/pub/png/libpng-manual.txt
  if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
    png_set_expand_gray_1_2_4_to_8(png_);
  }

  if(PNG_INFO_tRNS == png_get_valid(png_, info_, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_);
  }

  if(bitDepth == 16) {
    png_set_swap(png_);
  }

  Reader::Result result = {};

  if (PNG_INFO_sRGB == png_get_valid(png_, info_, PNG_INFO_sRGB)) {
    int intent = {};
    if(PNG_INFO_sRGB == png_get_sRGB(png_, info_, &intent)) {
      // indicate sRGB
      result.sRGB = std::monostate();
    }
  }

  if (PNG_INFO_iCCP == png_get_valid(png_, info_, PNG_INFO_iCCP)) {
    png_charp name = {};
    int compression_type = {};
    png_bytep profdata = {};
    png_uint_32 profLen = {};
    if(PNG_INFO_iCCP == png_get_iCCP(png_, info_, &name, &compression_type, &profdata, &profLen)) {
      result.iccProfile = std::vector<uint8_t>(profdata, profdata + profLen);
    }
  }

  if (PNG_INFO_cHRM == png_get_valid(png_, info_, PNG_INFO_cHRM)) {
    ColorPrimaries colorPrimaries = {};
    if (PNG_INFO_cHRM == png_get_cHRM_fixed(
        png_,
        info_,
        &colorPrimaries.whiteX,
        &colorPrimaries.whiteY,
        &colorPrimaries.redX,
        &colorPrimaries.redY,
        &colorPrimaries.greenX,
        &colorPrimaries.greenY,
        &colorPrimaries.blueX,
        &colorPrimaries.blueY)) {
      result.colorPrimaries = colorPrimaries;
    }
  }

  if (PNG_INFO_gAMA == png_get_valid(png_, info_, PNG_INFO_gAMA)) {
    png_fixed_point gamma = {};
    if (PNG_INFO_gAMA == png_get_gAMA_fixed(png_, info_, &gamma)) {
      result.gamma = gamma;
    }
  }

  avif::img::PixelOrder pixelOrder = {};
  switch(colorType) {
    case PNG_COLOR_TYPE_GRAY:
      pixelOrder = avif::img::PixelOrder::Mono;
      break;
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_PALETTE:
      pixelOrder = avif::img::PixelOrder::RGB;
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      pixelOrder = avif::img::PixelOrder::MonoA;
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      pixelOrder = avif::img::PixelOrder::RGBA;
      break;
    default:
      throw std::logic_error(fmt::format("Unknown bit depth: {}", bitDepth));
  }
  size_t const bytesPerPixel = avif::img::calcNumComponents(pixelOrder) * (bitDepth / 8);
  png_read_update_info(png_, info_);
  std::vector<uint8_t> data;
  data.resize(bytesPerPixel * width * height);
  std::vector<png_bytep> rows;
  rows.resize(height);
  for(uint32_t y = 0; y < height; y++) {
    rows.at(y) = std::next(data.data(), y * bytesPerPixel * width);
  }
  png_read_image(png_, rows.data());

  if(bitDepth == 16) {
    result.image = avif::img::Image<16>(avif::img::ColorProfile{}, pixelOrder, width, height, width * bytesPerPixel, std::move(data));
  } else {
    result.image = avif::img::Image<8>(avif::img::ColorProfile{}, pixelOrder, width, height, width * bytesPerPixel, std::move(data));
  }
  return result;
}

}
