#include <thread>

#include <aom/aom_encoder.h>
#include <aom/aom_codec.h>
#include <aom/aomcx.h>

#include <avif/av1/Parser.hpp>
#include <avif/util/FileLogger.hpp>
#include <avif/util/File.hpp>
#include <avif/FileBox.hpp>
#include <avif/Writer.hpp>
#include <avif/img/Image.hpp>

#include "Config.hpp"
#include "AVIFBuilder.hpp"
#include "img/PNGReader.hpp"
#include "img/Convertion.hpp"

namespace {

// FIXME(ledyba-z): remove this function when the C++20 comes.
bool endsWidh(std::string const& target, std::string const& suffix) {
  if(target.size() < suffix.size()) {
    return false;
  }
  return target.substr(target.size()-suffix.size()) == suffix;
}

size_t encode(avif::util::Logger& log, aom_codec_ctx_t& codec, aom_image* img, std::vector<std::vector<uint8_t>>& packets) {
  aom_codec_cx_pkt_t const* pkt;
  aom_codec_iter_t iter = nullptr;
  aom_codec_err_t const res = aom_codec_encode(&codec, img, 0, 1, img ? AOM_EFLAG_FORCE_KF : 0);
  if (res != AOM_CODEC_OK) {
    if(img) {
      log.fatal("failed to encode a frame: {}", aom_codec_error_detail(&codec));
    } else {
      log.fatal("failed to flush encoder: {}", aom_codec_error_detail(&codec));
    }
    return 0;
  }
  size_t numPackets = 0;
  while ((pkt = aom_codec_get_cx_data(&codec, &iter)) != nullptr) {
    if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
      auto& frame = pkt->data.frame;
      auto const beg = reinterpret_cast<uint8_t*>(frame.buf);
      packets.emplace_back(std::vector<uint8_t>(beg, beg + frame.sz));
      ++numPackets;
    }
  }
  return numPackets;
}

}

namespace internal{
int main(int argc, char** argv);
}
void printSequenceHeader(avif::util::Logger& log, avif::av1::SequenceHeader& seq);

int main(int argc, char** argv) {
  try {
    return internal::main(argc, argv);
  } catch (std::exception& err) {
    fprintf(stderr, "%s\n", err.what());
    fflush(stderr);
    return -1;
  }
}

int internal::main(int argc, char** argv) {
  avif::util::FileLogger log(stdout, stderr, avif::util::Logger::Level::DEBUG);
  log.info("cavif");
  log.info("libaom ver: {}", aom_codec_version_str());

  aom_codec_iface_t* av1codec = aom_codec_av1_cx();
  if(!av1codec) {
    log.fatal("failed to get AV1 encoder.");
  }

  Config config(argc, argv);
  aom_codec_enc_config_default(av1codec, &config.codec, AOM_USAGE_GOOD_QUALITY);
  // Set our default.
  config.codec.rc_end_usage = AOM_Q;
  config.codec.rc_target_bitrate = 0;
  config.codec.g_threads = std::thread::hardware_concurrency();
  {
    int const parsrResult = config.parse();
    if(parsrResult != 0) {
      return parsrResult;
    }
    if(config.showHelp) {
      config.usage();
      return 0;
    }
  }

  // decoding input image
  if(!endsWidh(config.input, ".png")) {
    log.fatal("please give png file for input");
  }
  std::variant<avif::img::Image<8>, avif::img::Image<16>> loadedImage = PNGReader(config.input).read();

  aom_image_t img;
  avif::img::ColorProfile colorProfile;
  if(std::holds_alternative<avif::img::Image<8>>(loadedImage)) {
    auto src = std::get<avif::img::Image<8>>(loadedImage);
    convert(config, src, img);
    colorProfile = src.colorProfile();
  } else {
    auto src = std::get<avif::img::Image<16>>(loadedImage);
    convert(config, src, img);
    colorProfile = src.colorProfile();
  }

  uint32_t const width = aom_img_plane_width(&img, AOM_PLANE_Y);
  uint32_t const height = aom_img_plane_height(&img, AOM_PLANE_Y);

  // initialize encoder
  config.codec.g_w = width;
  config.codec.g_h = height;
  // Generate just one frame.
  config.codec.g_limit = 1;
  config.codec.g_pass = AOM_RC_ONE_PASS;
  // FIXME(ledyba-z): Encoder produces wrong images when g_input_bit_depth != g_bit_depth. Bug?
  config.codec.g_input_bit_depth = config.codec.g_bit_depth;
  // FIXME(ledyba-z): If kf_max_dist = 1, it crashes. Bug?
  // > A value of 0 implies all frames will be keyframes.
  // However, when it is set to 0, assertion always fails:
  // cavif/external/libaom/av1/encoder/gop_structure.c:92:
  // construct_multi_layer_gf_structure: Assertion `gf_interval >= 1' failed.
  config.codec.kf_max_dist = 1;
  // One frame takes 1 second.
  config.codec.g_timebase.den = 1;
  config.codec.g_timebase.num = 1;

  aom_codec_ctx_t codec{};

  aom_codec_flags_t flags = 0;
  if(config.codec.g_bit_depth > 8) {
    flags = AOM_CODEC_USE_HIGHBITDEPTH;
  }
  if(AOM_CODEC_OK != aom_codec_enc_init(&codec, av1codec, &config.codec, flags)) {
    log.fatal("Failed to initialize encoder: {}", aom_codec_error_detail(&codec));
  }

  config.modify(&codec);

  std::vector<std::vector<uint8_t>> packets;
  {
    log.info("Encoding: {} -> {}", config.input, config.output);
    auto start = std::chrono::steady_clock::now();
    encode(log, codec, &img, packets);
    while(encode(log, codec, nullptr, packets) > 0); //flushing
    auto finish = std::chrono::steady_clock::now();
    log.info(" Encoded: {} -> {} in {:.2f} [sec]", config.input, config.output, std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count() / 1000.0f);
  }
  aom_img_free(&img);

  if (aom_codec_destroy(&codec) != AOM_CODEC_OK) {
    log.error("Failed to destroy codec: {}", aom_codec_error_detail(&codec));
    return -1;
  }

  if(packets.empty()) {
    log.error("no packats to out.");
    return -1;
  }
  {
    AVIFBuilder builder(log, config, width, height);
    std::shared_ptr<avif::av1::Parser::Result> result = avif::av1::Parser(log, packets[0]).parse();
    if (!result->ok()) {
      log.error(result->error());
      return -1;
    }
    std::optional<avif::av1::SequenceHeader> seq{};
    std::vector<uint8_t> configOBUs;
    std::vector<uint8_t> mdat;
    for(avif::av1::Parser::Result::Packet const& packet : result->packets()) {
      switch (packet.type()) {
        case avif::av1::Header::Type::TemporalDelimiter:
        case avif::av1::Header::Type::Padding:
        case avif::av1::Header::Type::Reserved:
          break;
        case avif::av1::Header::Type::SequenceHeader:
          seq = std::get<avif::av1::SequenceHeader>(packet.content());
          configOBUs.insert(std::end(configOBUs), std::next(std::begin(result->buffer()), packet.beg()), std::next(result->buffer().begin(), packet.end()));
          mdat.insert(std::end(mdat), std::next(std::begin(result->buffer()), packet.beg()), std::next(std::begin(result->buffer()), packet.end()));
          break;
        case avif::av1::Header::Type::Frame:
        default: {
          mdat.insert(std::end(mdat), std::next(std::begin(result->buffer()), packet.beg()), std::next(std::begin(result->buffer()), packet.end()));
          break;
        }
      }
    }
    if (!seq.has_value()) {
      throw std::logic_error("No sequence header OBU.");
    }
    builder.setPrimaryFrame(AVIFBuilder::Frame(colorProfile, seq.value(), std::move(configOBUs), std::move(mdat)));
    if(config.alphaInput.has_value()) {
      log.info("Attaching {} as Alpha plane.", config.alphaInput.value());
      builder.setAlphaFrame(AVIFBuilder::Frame::load(log, config.alphaInput.value()));
    }
    if(config.depthInput.has_value()) {
      log.info("Attaching {} as Depth plane.", config.depthInput.value());
      builder.setDepthFrame(AVIFBuilder::Frame::load(log, config.depthInput.value()));
    }
    std::vector<uint8_t> data = builder.build();
    std::optional<std::string> writeResult = avif::util::writeFile(config.output, data);
    if (writeResult.has_value()) {
      log.error(writeResult.value());
      return -1;
    }
    if (config.showResult) {
      printSequenceHeader(log, seq.value());
    }
  }
  return 0;
}

void printSequenceHeader(avif::util::Logger& log, avif::av1::SequenceHeader& seq) {
  log.info("<Encoding Result>");
  log.info(" - OBU Sequence Header:");
  log.info("   - AV1 Profile: {}", seq.seqProfile);
  log.info("   - Still picture: {}", seq.stillPicture ? "Yes" : "No");
  log.info("   - Reduced still picture header: {}", seq.reducedStillPictureHeader ? "Yes" : "No");
  log.info("   - Sequence Level Index at OperatingPoint[0]: {}", seq.operatingPoints.at(0).seqLevelIdx);
  log.info("   - Max frame width: {}", seq.maxFrameWidth);
  log.info("   - Max frame height: {}", seq.maxFrameHeight);
  log.info("   - Use 128x128 superblock: {}", seq.use128x128Superblock ? "Yes" : "No");
  log.info("   - FilterIntra enabled: {}", seq.enableFilterIntra ? "Yes" : "No");
  log.info("   - IntraEdgeFilter enabled: {}", seq.enableIntraEdgeFilter ? "Yes" : "No");
/*
  log.info("   - InterIntraCompound enabled: {}", seq.enableInterintraCompound ? "Yes" : "No");
  log.info("   - Masked Compound enabled: {}", seq.enableMaskedCompound ? "Yes" : "No");
  log.info("   - WarpedMotion enabled: {}", seq.enableWarpedMotion ? "Yes" : "No");
  log.info("   - DualFilter enabled: {}", seq.enableDualFilter ? "Yes" : "No");
  log.info("   - OrderHint enabled: {}", seq.enableOrderHint ? "Yes" : "No");
  log.info("   - JNTComp enabled: {}", seq.enableJNTComp ? "Yes" : "No");
  log.info("   - RefFrameMVS enabled: {}", seq.enableRefFrameMVS ? "Yes" : "No");
*/
  log.info("   - Superres enabled: {}", seq.enableSuperres ? "Yes" : "No");
  log.info("   - CDEF enabled: {}", seq.enableCDEF ? "Yes" : "No");
  log.info("   - Loop Restoration enabled: {}", seq.enableRestoration ? "Yes" : "No");
  log.info("   - Film Grain Params Present: {}", seq.filmGrainParamsPresent ? "Yes" : "No");
  log.info("   - Color Info:");
  log.info("     - High bit-depth: {}", seq.colorConfig.highBitdepth ? "Yes" : "No");
  log.info("     - Twelve bit: {}", seq.colorConfig.twelveBit ? "Yes" : "No");
  log.info("     - Monochrome: {}", seq.colorConfig.monochrome ? "Yes" : "No");
  log.info("     - Color primaries: {}", seq.colorConfig.colorPrimaries.has_value() ? std::to_string(static_cast<uint8_t>(seq.colorConfig.colorPrimaries.value())) : "<Unknownn>");
  log.info("     - Transfer characteristics: {}", seq.colorConfig.transferCharacteristics.has_value() ? std::to_string(static_cast<uint8_t>(seq.colorConfig.transferCharacteristics.value())) : "<Unknownn>");
  log.info("     - Matrix coefficients: {}", seq.colorConfig.matrixCoefficients.has_value() ? std::to_string(static_cast<uint8_t>(seq.colorConfig.matrixCoefficients.value())) : "<Unknownn>");
  log.info("     - Color range: {}", seq.colorConfig.colorRange ? "Full Ranged" : "Limited");
  log.info("     - Sub sampling X: {}", seq.colorConfig.subsamplingX);
  log.info("     - Sub sampling Y: {}", seq.colorConfig.subsamplingX);
  log.info("     - Chroma sample position: {}", seq.colorConfig.chromaSamplePosition.has_value() ? std::to_string(static_cast<uint8_t>(seq.colorConfig.chromaSamplePosition.value())) : "<Unknownn>");
  log.info("     - Separate UV Delta Q: {}", seq.colorConfig.separateUVDeltaQ ? "Yes" : "No");
}
