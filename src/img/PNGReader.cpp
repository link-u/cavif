//
// Created by psi on 2020/01/05.
//

#include "PNGReader.hpp"
#include <filesystem>
#include <system_error>
#include <fmt/format.h>

PNGReader::PNGReader(FILE *file, png_structp png, png_infop info)
:file_(file)
,png_(png)
,info_(info)
{
}

PNGReader PNGReader::create(std::string const& filename) {
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

  return PNGReader(file, png, info);
}

PNGReader::~PNGReader() noexcept {
  // Clean up reader
  png_destroy_read_struct(&png_, &info_, nullptr);
  // Close file
  fclose(file_);
}

std::variant<avif::img::Image<8>, avif::img::Image<16>> PNGReader::read() {
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

  avif::img::ColorProfile colorProfile;
  if (PNG_INFO_sRGB == png_get_valid(png_, info_, PNG_INFO_sRGB)) {
    int intent = {};
    if(PNG_INFO_sRGB == png_get_sRGB(png_, info_, &intent)) {
      // see H.273
      colorProfile = avif::ColourInformationBox::CICP {
          .colourPrimaries = 1,
          .transferCharacteristics = 13,
          .matrixCoefficients = 5,
          .fullRangeFlag = true,
      };
    }
  }

  if (PNG_INFO_iCCP == png_get_valid(png_, info_, PNG_INFO_iCCP)) {
    png_charp name = {};
    int compression_type = {};
    png_bytep profdata = {};
    png_uint_32 profLen = {};
    if(PNG_INFO_iCCP == png_get_iCCP(png_, info_, &name, &compression_type, &profdata, &profLen)) {
      std::vector<uint8_t> data(profdata, profdata + profLen);
      colorProfile = avif::img::ICCProfile(std::move(data));
    }
  }

  if (PNG_INFO_cHRM == png_get_valid(png_, info_, PNG_INFO_cHRM)) {
    png_fixed_point redX, redY, redZ;
    png_fixed_point greenX, greenY, greenZ;
    png_fixed_point blueX, blueY, blueZ;
    if (PNG_INFO_cHRM == png_get_cHRM_XYZ_fixed(png_, info_, &redX, &redY, &redZ, &greenX, &greenY, &greenZ, &blueX, &blueY, &blueZ)) {
      // TODO(ledyba-z): Support.
    }
  }

  int bytesPerPixel = {};
  avif::img::PixelOrder pixelOrder = {};
  switch(colorType) {
    case PNG_COLOR_TYPE_GRAY:
      pixelOrder = avif::img::PixelOrder::Mono;
      if(bitDepth == 16) {
        bytesPerPixel = 2;
      } else {
        bytesPerPixel = 1;
      }
      break;
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_PALETTE:
      pixelOrder = avif::img::PixelOrder::RGB;
      if(bitDepth == 16) {
        bytesPerPixel = 6;
      } else {
        bytesPerPixel = 3;
      }
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      pixelOrder = avif::img::PixelOrder::MonoA;
      if(bitDepth == 16) {
        bytesPerPixel = 4;
      } else {
        bytesPerPixel = 2;
      }
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      pixelOrder = avif::img::PixelOrder::RGBA;
      if(bitDepth == 16) {
        bytesPerPixel = 8;
      } else {
        bytesPerPixel = 4;
      }
      break;
    default:
      throw std::logic_error(fmt::format("Unknown bit depth: {}", bitDepth));
  }
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
    return avif::img::Image<16>(std::move(colorProfile), pixelOrder, width, height, bytesPerPixel, width * bytesPerPixel, std::move(data));
  } else {
    return avif::img::Image<8>(std::move(colorProfile), pixelOrder, width, height, bytesPerPixel, width * bytesPerPixel, std::move(data));
  }
}
