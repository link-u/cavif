//
// Created by psi on 2021/11/23.
//

#include "Encoder.hpp"

Encoder::Encoder(avif::util::FileLogger& log, Config& config, aom_codec_iface_t* interface, aom_codec_flags_t codec_flag, aom_image_t& image)
: log_(log)
, config_(config)
, interface(interface)
, codec_flag_(codec_flag)
, image_(image)
{

}

std::vector<std::vector<uint8_t>> Encoder::encodeInSinglePass() {
  aom_codec_ctx_t codec{};
  std::vector<std::vector<uint8_t>> packets;
  config_.codec.g_pass = AOM_RC_ONE_PASS;
  if(AOM_CODEC_OK != aom_codec_enc_init(&codec, interface, &config_.codec, codec_flag_)) {
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
    if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
      auto& frame = pkt->data.frame;
      auto const beg = reinterpret_cast<uint8_t*>(frame.buf);
      packets.emplace_back(std::vector<uint8_t>(beg, beg + frame.sz));
      ++numPackets;
    }
  }
  return numPackets;
}
