//
// Created by psi on 2020/01/05.
//

#include "PNGReader.hpp"
#include <filesystem>
#include <system_error>
#include <fmt/format.h>
#include <png.h>
#include <cassert>

#include "Image.hpp"

namespace img {
Image PNGReader::read() {
  FILE* file = fopen(filename_.c_str(), "rb");
  if(!file) {
    fclose(file);
    throw std::filesystem::filesystem_error("failed to open", this->filename_, std::make_error_code(static_cast<std::errc>(errno)));
  }
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if(!png) {
    fclose(file);
    throw std::runtime_error("failed to create png reader");
  }
  png_infop info = png_create_info_struct(png);
  if(!info) {
    fclose(file);
    throw std::runtime_error("failed to create png info structure");
  }
  png_init_io(png, file);
  png_read_info(png, info);
  int width = png_get_image_width(png, info);
  int height = png_get_image_height(png, info);
  png_byte color_type = png_get_color_type(png, info);
  png_byte bit_depth  = png_get_bit_depth(png, info);

  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt
  // FIXME(ledyba-z): av1 supports high-depth image.
  int bytesPerPixel = 3;
  if(bit_depth == 16) {
    png_set_strip_16(png);
  }

  if(color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png);
  }

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  // http://www.libpng.org/pub/png/libpng-manual.txt
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
    png_set_expand_gray_1_2_4_to_8(png);
  }

  if(png_get_valid(png, info, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png);
  }

  Image::Type type = Image::Type::RGB;
  // These color_type don't have an alpha channel then fill it with 0xff.
  switch(color_type) {
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_PALETTE:
      bytesPerPixel = 3;
      type = Image::Type::RGB;
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      bytesPerPixel = 4;
      type = Image::Type::RGBA;
      break;
    default:
      assert("Never to come here.");
  }
  switch(color_type) {
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_PALETTE:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      break;
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      png_set_gray_to_rgb(png);
      break;
    default:
      assert("Never to come here.");
  }
  png_read_update_info(png, info);
  std::vector<uint8_t> data;
  data.resize(bytesPerPixel * width * height);
  std::vector<png_bytep> rows;
  rows.resize(height);
  for(int y = 0; y < height; y++) {
    rows.at(y) = std::next(data.data(), y * bytesPerPixel * width);
  }
  png_read_image(png, rows.data());
  fclose(file);
  png_destroy_read_struct(&png, &info, nullptr);
  return Image(type, width, height, bytesPerPixel, width * bytesPerPixel, std::move(data));
}

}