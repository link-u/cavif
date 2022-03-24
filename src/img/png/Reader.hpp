//
// Created by psi on 2020/01/05.
//

#pragma once

#include <variant>
#include <string>
#include <png.h>
#include <aom/aom_image.h>
#include <avif/img/Image.hpp>
#include <avif/util/FileLogger.hpp>

namespace img::png {

struct ColorPrimaries final {
  png_fixed_point whiteX;
  png_fixed_point whiteY;
  png_fixed_point redX;
  png_fixed_point redY;
  png_fixed_point greenX;
  png_fixed_point greenY;
  png_fixed_point blueX;
  png_fixed_point blueY;
};

class Reader final {
public:
  struct Result final {
    std::variant<avif::img::Image<8>, avif::img::Image<16>> image;
    std::optional<std::monostate> sRGB;
    std::optional<ColorPrimaries> colorPrimaries;
    std::optional<png_fixed_point> gamma;
    std::optional<std::vector<uint8_t>> iccProfile;
  };
private:
  avif::util::FileLogger& log_;
  FILE* const file_;
  png_structp png_;
  png_infop info_;
public:
  Reader() = delete;
  Reader(Reader const&) = delete;
  Reader(Reader&&) = delete;
  Reader& operator=(Reader const&) = delete;
  Reader& operator=(Reader&&) = delete;
  ~Reader() noexcept;

public:
  static char const* version();
  static Reader create(avif::util::FileLogger& log, std::string const& filename);
  Result read();

private:
  explicit Reader(avif::util::FileLogger& log, FILE* file, png_structp png, png_infop info);
};

}
