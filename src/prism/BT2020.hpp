//
// Created by psi on 2020/01/21.
//

#pragma once

#include <cstdint>
#include "Image.hpp"

namespace prism {

// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2020-2-201510-I!!PDF-E.pdf
// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-6-201506-I!!PDF-E.pdf

class BT2020 final {
private:
  uint8_t const bits_;
  float const gamma_;
public:
  explicit BT2020(uint8_t bits, float gamma);
  void convertI444(Image const& src, uint16_t* dstY, size_t strideY, uint16_t* dstU, size_t strideU, uint16_t* dstV, size_t strideV);
  void convertI422(Image const& src, uint16_t* dstY, size_t strideY, uint16_t* dstU, size_t strideU, uint16_t* dstV, size_t strideV);
  void convertI420(Image const& src, uint16_t* dstY, size_t strideY, uint16_t* dstU, size_t strideU, uint16_t* dstV, size_t strideV);
  void convertI400(Image const& src, uint16_t* dstY, size_t strideY);

private:
  void calcYUV(uint16_t sr, uint16_t sg, uint16_t sb, float maxRGB, uint16_t* dstY, uint16_t* dstU, uint16_t* dstV);
};

}



