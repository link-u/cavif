//
// Created by psi on 2020/01/05.
//

#pragma once

#include <string>
#include <aom/aom_image.h>
#include <avif/img/Image.hpp>
#include <variant>

class PNGReader final {
private:
  std::string filename_;
public:
  PNGReader() = delete;
  PNGReader(PNGReader const&) = default;
  PNGReader(PNGReader&&) = default;
  PNGReader& operator=(PNGReader const&) = default;
  PNGReader& operator=(PNGReader&&) = default;
  ~PNGReader() noexcept = default;

public:
  explicit PNGReader(std::string filename)
  :filename_(std::move(filename))
  {
  }

public:
  std::variant<avif::img::Image<8>, avif::img::Image<16>> read();
};



