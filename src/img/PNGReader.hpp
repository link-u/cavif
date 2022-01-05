//
// Created by psi on 2020/01/05.
//

#pragma once

#include <string>
#include <png.h>
#include <aom/aom_image.h>
#include <avif/img/Image.hpp>
#include <variant>

class PNGReader final {
private:
  FILE* const file_;
  png_structp png_;
  png_infop info_;
public:
  PNGReader() = delete;
  PNGReader(PNGReader const&) = delete;
  PNGReader(PNGReader&&) = delete;
  PNGReader& operator=(PNGReader const&) = delete;
  PNGReader& operator=(PNGReader&&) = delete;
  ~PNGReader() noexcept;

public:
  static PNGReader create(std::string const& filename);
  std::variant<avif::img::Image<8>, avif::img::Image<16>> read();

private:
  explicit PNGReader(FILE* file, png_structp png, png_infop info);
};
