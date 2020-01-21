//
// Created by psi on 2020/01/21.
//

#pragma once

#include <cstdint>
#include <cmath>
#include "Image.hpp"

namespace prism {

// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2020-2-201510-I!!PDF-E.pdf
// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-6-201506-I!!PDF-E.pdf

namespace spec {

template <uint8_t bitsPerComponent> struct YUV;
template <> struct YUV<8> {
  using Type = uint8_t;
};
template <> struct YUV<10> {
  using Type = uint16_t;
};
template <> struct YUV<12> {
  using Type = uint16_t;
};

template <uint8_t bitsPerComponent> struct RGB;
template <> struct RGB<8> {
  using Type = uint8_t;
  static constexpr float max = 255.0f;
};
template <> struct RGB<16> {
  using Type = uint16_t;
  static constexpr float max = 65535.0f;
};

}

namespace detail{

constexpr int clamp(int value, int low, int high) {
  return value < low ? low : (value > high ? high : value);
}

template <size_t rgbBits, size_t yuvBits>
constexpr void calcYUV(uint16_t const sr, uint16_t const sg, uint16_t const sb, typename spec::YUV<yuvBits>::Type* dstY, typename spec::YUV<yuvBits>::Type* dstU, typename spec::YUV<yuvBits>::Type* dstV) {
  using YUVType = typename spec::YUV<yuvBits>::Type;
  using RGBType = typename spec::RGB<rgbBits>::Type;
  float const r = static_cast<float>(sr) / spec::RGB<rgbBits>::max;
  float const g = static_cast<float>(sg) / spec::RGB<rgbBits>::max;
  float const b = static_cast<float>(sb) / spec::RGB<rgbBits>::max;

  auto constexpr shift = static_cast<float>(1u << (yuvBits - 8u));
  int constexpr maxYUV = (1u << yuvBits) - 1u;
  float const y = 0.2627f * r + 0.6780f * g + 0.0593f * b;
  *dstY = static_cast<YUVType>(clamp(static_cast<int>(std::round(y * 219 + 16) * shift), 0, maxYUV));

  if (dstU) {
    float const u = (b - y) / 1.8814f;
    *dstU = static_cast<YUVType>(clamp(static_cast<int>(std::round(u * 224 + 128) * shift), 0, maxYUV));
  }
  if (dstV) {
    float const v = (r - y) / 1.4746f;
    *dstV = static_cast<YUVType>(clamp(static_cast<int>(std::round(v * 224 + 128) * shift), 0, maxYUV));
  }
}

template <uint8_t rgbBits, uint8_t yuvBits, bool subX, bool subY>
void constexpr convert(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t const* src, size_t stride, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV) {
  using YUVType = typename spec::YUV<yuvBits>::Type;
  using RGBType = typename spec::RGB<rgbBits>::Type;
  uint8_t* lineY = dstY;
  uint8_t* lineU = dstU;
  uint8_t* lineV = dstV;
  uint8_t const* line = src;
  for(size_t y = 0; y < height; ++y) {
    uint8_t const *ptr = line;
    auto* ptrY = reinterpret_cast<YUVType*>(lineY);
    auto* ptrU = reinterpret_cast<YUVType*>(lineU);
    auto* ptrV = reinterpret_cast<YUVType*>(lineV);
    for (size_t x = 0; x < width; ++x) {
      uint16_t const r = reinterpret_cast<RGBType const *>(ptr)[0];
      uint16_t const g = reinterpret_cast<RGBType const *>(ptr)[1];
      uint16_t const b = reinterpret_cast<RGBType const *>(ptr)[2];
      if (subX) {
        if(x % 2 == 0) {
          calcYUV<rgbBits, yuvBits>(r, g, b, &ptrY[x], &ptrU[x/2], &ptrV[x/2]);
        } else {
          calcYUV<rgbBits, yuvBits>(r, g, b, &ptrY[x], nullptr, nullptr);
        }
      } else {
        calcYUV<rgbBits, yuvBits>(r, g, b, &ptrY[x], &ptrU[x], &ptrV[x]);
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    if(subY){
      if(y % 2 == 1) {
        lineU += strideU;
        lineV += strideV;
      }
    } else {
      lineU += strideU;
      lineV += strideV;
    }
    line += stride;
  }
}

}


template <uint8_t rgbBits, uint8_t yuvBits>
struct FromRGB final {
  void toI444(Image const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV) {
    detail::convert<rgbBits, yuvBits, false, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  void toI422(Image const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convert<rgbBits, yuvBits, true, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  void toI420(Image const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convert<rgbBits, yuvBits, true, true>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
};

}



