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

class Configurator {
public:
  std::string input{};
  std::string output{};
  aom_codec_enc_cfg encoderConfig{};
public:
  int crf = 32;
  int cpu_used = 1;
  bool enableCDEF = false;
  bool fullColorRange = false;
  aom_superblock_size_t superblockSize = AOM_SUPERBLOCK_SIZE_DYNAMIC;
  aom_tune_metric tune = AOM_TUNE_SSIM;
public:
  Configurator() = default;
  int parse(int argc, char** argv);
  void modify(aom_codec_ctx_t* codec);
};


