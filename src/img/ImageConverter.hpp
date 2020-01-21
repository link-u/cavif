//
// Created by psi on 2020/01/18.
//

#pragma once

#include <aom/aom_image.h>
#include <vector>
#include "../prism/Image.hpp"

class ImageConverter {
public:
  ImageConverter() = delete;
  ImageConverter(ImageConverter const&) = delete;
  ImageConverter(ImageConverter&&) = delete;
  ImageConverter& operator=(ImageConverter const&) = delete;
  ImageConverter& operator=(ImageConverter&&) = delete;

private:
  size_t const width_;
  size_t const height_;
  prism::Image& src_;
  aom_image& dst_;
public:
  explicit ImageConverter(prism::Image& src, aom_image& dst);
  void convert(uint8_t bitsPerComponent);
private:
  void convertFromBGRA8888(std::vector<uint8_t> const& bgra, const size_t stride);
};


