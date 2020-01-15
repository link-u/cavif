//
// Created by psi on 2020/01/05.
//

#include "Image.hpp"
namespace img {

Image::Image(Type type, uint32_t width, uint32_t height, uint32_t bytesPerPiexl, uint32_t stride, std::vector<uint8_t> data)
:type_(type)
,width_(width)
,height_(height)
,bytesPerPixel_(bytesPerPiexl)
,stride_(stride)
,data_(std::move(data))
{

}

}