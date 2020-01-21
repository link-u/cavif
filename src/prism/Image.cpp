//
// Created by psi on 2020/01/05.
//

#include "Image.hpp"

namespace prism {

Image::Image(Type type, uint32_t width, uint32_t height, uint8_t bitsPerComponent, uint32_t bytesPerPiexl, uint32_t stride, std::vector<uint8_t> data)
:type_(type)
,width_(width)
,height_(height)
,bitsPerComponent_(bitsPerComponent)
,bytesPerPixel_(bytesPerPiexl)
,stride_(stride)
,data_(std::move(data))
{

}

template <>
[[ nodiscard ]] uint8_t const* Image::data<uint8_t>() const {
  return this->data_.data();
}
template <>
[[ nodiscard ]] uint16_t const* Image::data<uint16_t>() const {
  return reinterpret_cast<uint16_t const*>(this->data_.data());
}

}