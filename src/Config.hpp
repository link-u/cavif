//
// Created by psi on 2020/01/14.
//

#pragma once

#include <string>
#include <optional>
#include <aom/aom_encoder.h>
#include <aom/aom_codec.h>
#include <aom/aomcx.h>
#include <avif/ImageRotationBox.hpp>
#include <avif/ImageMirrorBox.hpp>
#include <av1/encoder/encoder.h>
#include "../external/clipp/include/clipp.h"

class Configurator;
class Config final {
  friend class Configurator;
public:
  std::string input{};
  std::string output{};
public:
  // meta
  std::optional<avif::ImageRotationBox::Rotation> rotation{};
  std::optional<avif::ImageMirrorBox::Axis> mirrorAxis{};
  std::optional<std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>>> cropSize{};
  std::optional<std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>>> cropOffset{};
  // encoding
  aom_codec_enc_cfg codec{};
  aom_img_fmt_t pixFmt = AOM_IMG_FMT_I420;
  int crf = 32;
  bool useQM;
  int qmMin = DEFAULT_QM_FIRST;
  int qmMax = DEFAULT_QM_LAST;
  int qmMinY = DEFAULT_QM_Y;
  int qmMinU = DEFAULT_QM_U;
  int qmMinV = DEFAULT_QM_V;
  bool enableRectPartition = true;
  bool enableABPartition = true;
  bool enable1to4Partition = true;
  int minPartitionSize = 4;
  int maxPartitionSize = 128;
  bool rowMT = false;
  int cpuUsed = 1;
  int sharpness = 0;
  int tileRows = 0;
  int tileColums = 0;
  bool enableKeyframeTemporalFiltering = false;
  bool enableIntraEdgeFilter = true;
  bool enableTX64 = true;
  bool enableFlipIDTX = true;
  int adaptiveQuantization = NO_AQ;
  bool lossless = false;
  bool enableCDEF = false;
  bool enableRestoration = false;
  bool fullColorRange = false;
  aom_superblock_size_t superblockSize = AOM_SUPERBLOCK_SIZE_DYNAMIC;
  aom_tune_metric tune = AOM_TUNE_SSIM;
public:
  Config() = default;
  Config(Config&&) noexcept = default;
  Config(Config const&) = default;
  Config& operator=(Config&&) noexcept = default;
  Config& operator=(Config const&) = default;

public:
  int parse(int argc, char** argv);
  void modify(aom_codec_ctx_t* aom);
};


