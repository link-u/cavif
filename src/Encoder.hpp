//
// Created by psi on 2021/11/23.
//

#pragma once

#include <avif/util/FileLogger.hpp>
#include "Config.hpp"

class Encoder {
public:
  Encoder() = delete;
  Encoder(Encoder const&) = delete;
  Encoder(Encoder&&) = delete;
  Encoder& operator=(Encoder const&) = delete;
  Encoder& operator=(Encoder&&) = delete;
public:
  Encoder(avif::util::FileLogger& log, Config& config, aom_codec_iface_t* interface, aom_codec_flags_t codec_flag, aom_image_t& image);

  std::vector<std::vector<uint8_t>> encodeInSinglePass();
  std::vector<std::vector<uint8_t>> encodeInTwoPass();

private:
  size_t collectStats(aom_codec_ctx_t* codec, aom_image* img, aom_fixed_buf_t *stats);
  size_t encodeFrame(aom_codec_ctx_t* codec, aom_image* img, std::vector<std::vector<uint8_t>>& packets);
private:
  avif::util::FileLogger& log_;
  Config& config_;
  aom_codec_iface_t* interface;
  aom_codec_flags_t const codecFlag_;
  aom_image_t& image_;
};
