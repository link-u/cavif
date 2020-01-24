//
// Created by psi on 2020/01/14.
//

#pragma once

#include <string>
#include <optional>
#include <aom/aom_encoder.h>
#include <aom/aom_codec.h>
#include <aom/aomcx.h>
#include "../external/clipp/include/clipp.h"

class Configurator;
class Config final {
  friend class Configurator;
public:
  enum class Rotation : uint8_t {
    Rot0   = 0,
    Rot90  = 1,
    Rot180 = 2,
    Rot270 = 3,
  };
public:
  std::string input{};
  std::string output{};
public:
  // meta
  std::optional<Rotation> rotation{};
  // encoding
  aom_codec_enc_cfg codec{};
  aom_img_fmt_t pixFmt = AOM_IMG_FMT_I420;
  int crf = 32;
  int cpuUsed = 1;
  bool enableCDEF = false;
  bool enableRestoration = false;
  bool fullColorRange = false;
  aom_superblock_size_t superblockSize = AOM_SUPERBLOCK_SIZE_DYNAMIC;
  aom_tune_metric tune = AOM_TUNE_SSIM;
public:
  Config() = default;
  Config(Config&&) = default;
  Config(Config const&) = default;
  Config& operator=(Config&&) = default;
  Config& operator=(Config const&) = default;

public:
  int parse(int argc, char** argv);
  void modify(aom_codec_ctx_t* codec);
};


