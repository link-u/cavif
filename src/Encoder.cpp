//
// Created by psi on 2021/11/23.
//

#include <chrono>
#include "Encoder.hpp"

Encoder::Encoder(avif::util::FileLogger& log, Config& config, aom_codec_iface_t* interface, aom_codec_flags_t codec_flag, aom_image_t& image)
: log_(log)
, config_(config)
, interface(interface)
, codecFlag_(codec_flag)
, image_(image)
{

}

std::vector<std::vector<uint8_t>> Encoder::encodeInSinglePass() {
  aom_codec_ctx_t codec{};
  std::vector<std::vector<uint8_t>> packets;
  config_.codec.g_pass = AOM_RC_ONE_PASS;
  if(AOM_CODEC_OK != aom_codec_enc_init(&codec, interface, &config_.codec, codecFlag_)) {
    log_.fatal("Failed to initialize encoder: {}", aom_codec_error_detail(&codec));
  }
  config_.modify(&codec);

  this->encodeFrame(&codec, &image_, packets);
  while(this->encodeFrame(&codec, nullptr, packets) > 0); //flush

  if (aom_codec_destroy(&codec) != AOM_CODEC_OK) {
    log_.fatal("Failed to destroy codec: {}", aom_codec_error_detail(&codec));
    // no return
  }
  return std::move(packets);
}

std::vector<std::vector<uint8_t>> Encoder::encodeInTwoPass() {
  // Refer: external/libaom/examples/twopass_encoder.c
  aom_fixed_buf_t stats = {.buf = nullptr, .sz = 0};
  { // First pass
    auto start = std::chrono::steady_clock::now();
    this->config_.codec.g_pass = AOM_RC_FIRST_PASS;
    aom_codec_ctx_t codec{};
    if(AOM_CODEC_OK != aom_codec_enc_init(&codec, interface, &config_.codec, codecFlag_)) {
      log_.fatal("Failed to initialize encoder: {}", aom_codec_error_detail(&codec));
    }
    config_.modify(&codec);

    this->collectStats(&codec, &image_, &stats);
    while(this->collectStats(&codec, nullptr, &stats) > 0); //flush

    if (aom_codec_destroy(&codec) != AOM_CODEC_OK) {
      log_.fatal("Failed to destroy codec: {}", aom_codec_error_detail(&codec));
      // no return
    }
    auto finish = std::chrono::steady_clock::now();
    auto elapsed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count()) / 1000.0f;
    log_.info("  Pass1 elapsed: {:.2f} [sec]", elapsed);
  }
  std::vector<std::vector<uint8_t>> packets;
  { // Last pass
    auto start = std::chrono::steady_clock::now();
    this->config_.codec.g_pass = AOM_RC_LAST_PASS;
    this->config_.codec.rc_twopass_stats_in = stats;

    aom_codec_ctx_t codec{};
    if(AOM_CODEC_OK != aom_codec_enc_init(&codec, interface, &config_.codec, codecFlag_)) {
      log_.fatal("Failed to initialize encoder: {}", aom_codec_error_detail(&codec));
    }
    config_.modify(&codec);

    this->encodeFrame(&codec, &image_, packets);
    while(this->encodeFrame(&codec, nullptr, packets) > 0); //flush

    if (aom_codec_destroy(&codec) != AOM_CODEC_OK) {
      log_.fatal("Failed to destroy codec: {}", aom_codec_error_detail(&codec));
      // no return
    }
    free(stats.buf);
    auto finish = std::chrono::steady_clock::now();
    auto elapsed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count()) / 1000.0f;
    log_.info("  Pass2 elapsed: {:.2f} [sec]", elapsed);
  }
  return std::move(packets);
}

size_t Encoder::collectStats(aom_codec_ctx_t* codec, aom_image* img, aom_fixed_buf_t *stats) {
  aom_codec_cx_pkt_t const* pkt;
  aom_codec_iter_t iter = nullptr;
  aom_codec_err_t const res = aom_codec_encode(codec, img, 0, 1, img ? AOM_EFLAG_FORCE_KF : 0);
  if (res != AOM_CODEC_OK) {
    if(img) {
      log_.fatal("failed to encode a frame: {}", aom_codec_error_detail(codec));
    } else {
      log_.fatal("failed to flush encoder: {}", aom_codec_error_detail(codec));
    }
    // no return
  }
  size_t numPackets = 0;
  while ((pkt = aom_codec_get_cx_data(codec, &iter)) != nullptr) {
    ++numPackets;
    if (pkt->kind == AOM_CODEC_STATS_PKT) {
      const void *const pkt_buf = pkt->data.twopass_stats.buf;
      const size_t pkt_size = pkt->data.twopass_stats.sz;
      stats->buf = realloc(stats->buf, stats->sz + pkt_size);
      memcpy(reinterpret_cast<uint8_t *>(stats->buf) + stats->sz, pkt_buf, pkt_size);
      stats->sz += pkt_size;
    }
  }
  return numPackets;
}

size_t Encoder::encodeFrame(aom_codec_ctx_t* codec, aom_image* img, std::vector<std::vector<uint8_t>>& packets) {
  aom_codec_cx_pkt_t const* pkt;
  aom_codec_iter_t iter = nullptr;
  aom_codec_err_t const res = aom_codec_encode(codec, img, 0, 1, img ? AOM_EFLAG_FORCE_KF : 0);
  if (res != AOM_CODEC_OK) {
    if(img) {
      log_.fatal("failed to encode a frame: {}", aom_codec_error_detail(codec));
    } else {
      log_.fatal("failed to flush encoder: {}", aom_codec_error_detail(codec));
    }
    // no return
  }
  size_t numPackets = 0;
  while ((pkt = aom_codec_get_cx_data(codec, &iter)) != nullptr) {
    ++numPackets;
    if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
      auto& frame = pkt->data.frame;
      auto const beg = reinterpret_cast<uint8_t*>(frame.buf);
      packets.emplace_back(std::vector<uint8_t>(beg, beg + frame.sz));
    }
  }
  return numPackets;
}
