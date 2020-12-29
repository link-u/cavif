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
  std::string commandName{};
  bool showHelp = false;
public:
  std::string input{};
  std::optional<std::string> alphaInput{};
  std::optional<std::string> depthInput{};
  std::string output{};
  enum EncodeTarget {
    Image,
    Alpha
  };
  EncodeTarget encodeTarget = EncodeTarget::Image;
  bool showResult = false;
public:
  // meta
  std::optional<avif::ImageRotationBox::Rotation> rotation{};
  std::optional<avif::ImageMirrorBox::Axis> mirrorAxis{};
  std::optional<std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>>> cropSize{};
  std::optional<std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>>> cropOffset{};
  // color
  uint8_t colorPrimaries = 1;
  uint8_t transferCharacteristics = 13;
  uint8_t matrixCoefficients = 1;
  // encoding
  aom_codec_enc_cfg codec{};
  aom_scaling_mode_t scaleMode = {
      .h_scaling_mode = AOME_NORMAL,
      .v_scaling_mode = AOME_NORMAL,
  };
  int renderWidth = 0;
  int renderHeight = 0;
  aom_img_fmt_t pixFmt = AOM_IMG_FMT_I420;
  int crf = 10;
  int deltaQMode = 0;
  bool enableChromaDeltaQ = false;
  bool enableDeltaLoopfilter = false;
  bool useQM = false;
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
  int tileColumns = 0;
  int keyframeTemporalFilter = 0;
  bool enableIntraEdgeFilter = true;
  bool enableTX64 = true;
  bool enableFlipIDTX = true;
  //bool enableRectTX = true;
  bool useDCTOnly = false;
  bool useDefaultTXOnly = false;
  bool useReducedTXSet = false;
  int adaptiveQuantizationMode = NO_AQ;
  bool enableAdaptiveQuantizationB = false;
  bool enableFilterIntra = true;
  bool enableSmoothIntra = true;
  bool enablePaethIntra = true;
  bool enableChromaFromLuma = true;
  bool enableSuperres = true;
  bool enablePalette = false;
  bool enableIntraBC = true;
  bool enableAngleDelta = true;
  bool lossless = false;
  bool enableCDEF = false;
  bool enableRestoration = false;
  bool fullColorRange = false;
  aom_superblock_size_t superblockSize = AOM_SUPERBLOCK_SIZE_DYNAMIC;
  aom_tune_metric tune = AOM_TUNE_SSIM;
  std::string vmafModelPath = "/usr/share/cavif/model/vmaf_v0.6.1.pkl";
public:
  Config() = delete;
  Config(Config&&) noexcept = default;
  Config(Config const&) = default;
  Config& operator=(Config&&) noexcept = default;
  Config& operator=(Config const&) = default;
  Config(int argc, char** argv);

private:
  int argc{};
  char** argv{};
  clipp::group commandLineFlags{};
  clipp::group createCommandLineFlags();
public:
  void usage();
  int parse();
  void modify(aom_codec_ctx_t* aom);
};


