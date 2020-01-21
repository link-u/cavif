//
// Created by psi on 2020/01/21.
//

#include <cmath>
#include "BT2020.hpp"

namespace {

int clamp(int value, int low, int high) {
  return value < low ? low : (value > high ? high : value);
}

}

namespace prism {

BT2020::BT2020(uint8_t const bits, float const gamma)
:bits_(bits)
,gamma_(gamma)
{

}

void BT2020::calcYUV(uint16_t sr, uint16_t sg, uint16_t sb, float maxRGB, uint16_t* dstY, uint16_t* dstU, uint16_t* dstV) {
  float const r = std::pow(static_cast<float>(sr) / maxRGB, 1.0f);
  float const g = std::pow(static_cast<float>(sg) / maxRGB, 1.0f);
  float const b = std::pow(static_cast<float>(sb) / maxRGB, 1.0f);

  auto const shift = static_cast<float>(1u << (bits_ - 8u));
  int maxYUV = (1u << bits_) - 1u;
  float const y = 0.2627f * r + 0.6780f * g + 0.0593f * b;
  *dstY = static_cast<uint16_t>(clamp(static_cast<int>(std::round(y * 219 + 16) * shift), 0, maxYUV));

  if (dstU) {
    float const u = (b - y) / 1.8814f;
    *dstU = static_cast<uint16_t>(clamp(static_cast<int>(std::round(u * 224 + 128) * shift), 0, maxYUV));
  }
  if (dstV) {
    float const v = (r - y) / 1.4746f;
    *dstV = static_cast<uint16_t>(clamp(static_cast<int>(std::round(v * 224 + 128) * shift), 0, maxYUV));
  }
}

void BT2020::convertI444(Image const& src, uint16_t* dstY, size_t strideY, uint16_t* dstU, size_t strideU, uint16_t* dstV, size_t strideV) {
  uint16_t* lineY = dstY;
  uint16_t* lineU = dstU;
  uint16_t* lineV = dstV;
  auto* lineRGB = src.data<uint8_t>();
  auto const maxRGB = static_cast<float>((1u << src.bitsPerComponent())- 1);
  for(size_t y = 0; y < src.height(); ++y) {
    uint8_t const* ptrRGB = lineRGB;
    for(size_t x = 0; x < src.width(); ++x) {
      u_int16_t r = (src.bitsPerComponent() == 8) ? ptrRGB[0] : reinterpret_cast<uint16_t const*>(ptrRGB)[0];
      u_int16_t g = (src.bitsPerComponent() == 8) ? ptrRGB[1] : reinterpret_cast<uint16_t const*>(ptrRGB)[1];
      u_int16_t b = (src.bitsPerComponent() == 8) ? ptrRGB[2] : reinterpret_cast<uint16_t const*>(ptrRGB)[2];
      calcYUV(r, g, b, maxRGB, &lineY[x], &lineU[x], &lineV[x]);

      ptrRGB += src.bytesPerPixel();
    }
    lineY = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineY) + strideY);
    lineU = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineU) + strideU);
    lineV = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineV) + strideV);
    lineRGB = lineRGB + src.stride();
  }
}

void BT2020::convertI422(Image const& src, uint16_t* dstY, size_t strideY, uint16_t* dstU, size_t strideU, uint16_t* dstV, size_t strideV) {
  uint16_t* lineY = dstY;
  uint16_t* lineU = dstU;
  uint16_t* lineV = dstV;
  auto* lineRGB = src.data<uint8_t>();
  auto const maxRGB = static_cast<float>((1u << src.bitsPerComponent())- 1);
  for(size_t y = 0; y < src.height(); ++y) {
    uint8_t const* ptrRGB = lineRGB;
    for(size_t x = 0; x < src.width(); ++x) {
      uint16_t r = (src.bitsPerComponent() == 8) ? ptrRGB[0] : reinterpret_cast<uint16_t const*>(ptrRGB)[0];
      uint16_t g = (src.bitsPerComponent() == 8) ? ptrRGB[1] : reinterpret_cast<uint16_t const*>(ptrRGB)[1];
      uint16_t b = (src.bitsPerComponent() == 8) ? ptrRGB[2] : reinterpret_cast<uint16_t const*>(ptrRGB)[2];
      if(x % 2 == 0) {
        calcYUV(r, g, b, maxRGB, &lineY[x], &lineU[x/2], &lineV[x/2]);
      } else {
        calcYUV(r, g, b, maxRGB, &lineY[x], nullptr, nullptr);
      }

      ptrRGB += src.bytesPerPixel();
    }
    lineY = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineY) + strideY);
    lineU = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineU) + strideU);
    lineV = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineV) + strideV);
    lineRGB = lineRGB + src.stride();
  }
}
void BT2020::convertI420(Image const& src, uint16_t* dstY, size_t strideY, uint16_t* dstU, size_t strideU, uint16_t* dstV, size_t strideV) {
  uint16_t* lineY = dstY;
  uint16_t* lineU = dstU;
  uint16_t* lineV = dstV;
  auto* lineRGB = src.data<uint8_t>();
  auto const maxRGB = static_cast<float>((1u << src.bitsPerComponent())- 1);
  for(size_t y = 0; y < src.height(); ++y) {
    uint8_t const* ptrRGB = lineRGB;
    for(size_t x = 0; x < src.width(); ++x) {
      u_int16_t r = (src.bitsPerComponent() == 8) ? ptrRGB[0] : reinterpret_cast<uint16_t const*>(ptrRGB)[0];
      u_int16_t g = (src.bitsPerComponent() == 8) ? ptrRGB[1] : reinterpret_cast<uint16_t const*>(ptrRGB)[1];
      u_int16_t b = (src.bitsPerComponent() == 8) ? ptrRGB[2] : reinterpret_cast<uint16_t const*>(ptrRGB)[2];
      if(x % 2 == 0) {
        calcYUV(r, g, b, maxRGB, &lineY[x], &lineU[x/2], &lineV[x/2]);
      } else {
        calcYUV(r, g, b, maxRGB, &lineY[x], nullptr, nullptr);
      }
      ptrRGB += src.bytesPerPixel();
    }
    lineY = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineY) + strideY);
    if(y % 2 == 1) {
      lineU = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineU) + strideU);
      lineV = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineV) + strideV);
    }
    lineRGB = lineRGB + src.stride();
  }
}
void BT2020::convertI400(Image const& src, uint16_t* dstY, size_t strideY) {
  uint16_t* lineY = dstY;
  auto* lineRGB = src.data<uint8_t>();
  auto const maxRGB = static_cast<float>((1u << src.bitsPerComponent())- 1);
  for(size_t y = 0; y < src.height(); ++y) {
    uint8_t const* ptrRGB = lineRGB;
    for(size_t x = 0; x < src.width(); ++x) {
      u_int16_t r = (src.bitsPerComponent() == 8) ? ptrRGB[0] : reinterpret_cast<uint16_t const*>(ptrRGB)[0];
      u_int16_t g = (src.bitsPerComponent() == 8) ? ptrRGB[1] : reinterpret_cast<uint16_t const*>(ptrRGB)[1];
      u_int16_t b = (src.bitsPerComponent() == 8) ? ptrRGB[2] : reinterpret_cast<uint16_t const*>(ptrRGB)[2];
      calcYUV(r, g, b, maxRGB, &lineY[x], nullptr, nullptr);
      ptrRGB += src.bytesPerPixel();
    }
    lineY = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(lineY) + strideY);
    lineRGB = lineRGB + src.stride();
  }
}

}
