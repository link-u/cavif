//
// Created by psi on 2020/01/14.
//

#include <iostream>

#include "Config.hpp"

namespace {

std::string basename(std::string const& path) {
  auto pos = path.find_last_of('/');
  if(pos == std::string::npos) {
    return path;
  }
  return path.substr(pos+1);
}

std::string trim(std::string str) {
  str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
    return !std::isspace(ch);
  }).base(), str.end());
  str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
    return !std::isspace(ch);
  }));
  return std::move(str);
}

std::pair<int32_t, uint32_t> parseFraction(std::string const& str) {
  auto pos = str.find('/');
  if(pos == std::string::npos) {
    return std::make_pair(std::stoi(trim(str)), 1);
  } else {
    std::string first = trim(str.substr(0, pos));
    std::string second = trim(str.substr(pos + 1));
    int n = std::stoi(first);
    int d = std::stoi(second);
    if(d == 0) {
      throw std::invalid_argument("denominator can't be 0.");
    }
    return std::make_pair(static_cast<int32_t>(n), static_cast<uint32_t>(d));
  }
}

std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>> parseFractionPair(std::string const& str) {
  auto pos = str.find(',');
  if(pos == std::string::npos) {
    throw std::invalid_argument(R"(Invalid fraction pair. Example: "30/4, 100/7", "100, 100/2" or "100, 100")");
  }
  std::string first = trim(str.substr(0, pos));
  std::string second = trim(str.substr(pos + 1));
  return std::make_pair(parseFraction(first), parseFraction(second));
}

}

int Config::parse(int argc, char **argv) {
  using namespace clipp;
  auto& aom = this->codec;
  auto cli = (
      required("-i", "--input").doc("Filename to input") & value("input.png", input),
      required("-o", "--output").doc("Filename to output") & value("output.avif", output),
      // meta
      option("--rotation").doc("Set rotation meta data(irot). Counter-clockwise.") & (parameter("0").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot0)) | parameter("90").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot90)) | parameter("180").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot180)) | parameter("270").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot270))),
      option("--mirror").doc("Set mirror meta data(imir).") & (parameter("vertical").set(mirrorAxis, std::make_optional(avif::ImageMirrorBox::Axis::Vertical)) | parameter("horizontal").set(mirrorAxis, std::make_optional(avif::ImageMirrorBox::Axis::Horizontal))),
      option("--crop-size").doc("Set crop size.") & value("widthN/widthD,heightN/heightD").call([&](std::string const& str){ cropSize = parseFractionPair(str); }),
      option("--crop-offset").doc("Set crop offset.") & value("horizOffN/horizOffD,vertOffN/vertOffD").call([&](std::string const& str){ cropOffset = parseFractionPair(str); }),

      // av1 sequence header
      option("--full-still-picture-header").doc("Force to output full picture header").set(aom.full_still_picture_hdr, 1u),

      // profile and pixel formats
      option("--profile").doc("AV1 Profile(0=base, 1=high, 2=professional)") & integer("0=base, 1=high, 2=professional", aom.g_profile),
      option("--pix-fmt").doc("Pixel format of output image.") & (parameter("yuv420").set(pixFmt, AOM_IMG_FMT_I420) | parameter("yuv422").set(pixFmt, AOM_IMG_FMT_I422) | parameter("yuv444").set(pixFmt, AOM_IMG_FMT_I444)),
      option("--bit-depth").doc("Bit depth of output image.") & (parameter("8").set(aom.g_bit_depth, AOM_BITS_8) | parameter("10").set(aom.g_bit_depth, AOM_BITS_10) | parameter("12").set(aom.g_bit_depth, AOM_BITS_12)),
      option("--disable-full-color-range").doc("Use limited YUV color range.").set(fullColorRange, false),
      option("--enable-full-color-range").doc("Use full YUV color range.").set(fullColorRange, true),

      // trade offs between speed and quality.
      option("--encoder-usage").doc("Encoder usage") & (parameter("good").doc("Good Quality mode").set(aom.g_usage, static_cast<unsigned int>(AOM_USAGE_GOOD_QUALITY)) | parameter("realtime").doc("Real time encoding mode.").set(aom.g_usage, static_cast<unsigned int>(AOM_USAGE_REALTIME))),
      option("--threads") & integer("Num of threads to use", aom.g_threads),
      option("--row-mt").doc("Enable row based multi-threading of encoder").set(rowMT, true),
      option("--cpu-used").doc("Quality/Speed ratio modifier") & integer("0-8", cpuUsed),

      // rate-control
      option("--rate-control").doc("Rate control method") & (parameter("q").doc("Constant Quality").set(aom.rc_end_usage, AOM_Q) | parameter("cq").doc("Constrained Quality").set(aom.rc_end_usage, AOM_CQ)),
      option("--crf").doc("CQ Level in CQ rate control mode") & integer("0-63", crf),
      option("--bit-rate").doc("Bit rate of output image.") & integer("kilo-bits per second", aom.rc_target_bitrate),
      option("--tune").doc("Quality metric to tune") & (parameter("psnr").doc("peak signal-to-noise ratio").set(tune, AOM_TUNE_PSNR) | parameter("ssim").doc("structural similarity").set(tune, AOM_TUNE_SSIM) | parameter("cdef-dist").doc("cdef-dist").set(tune, AOM_TUNE_CDEF_DIST) | parameter("daala-dist").doc("daala-dist").set(tune, AOM_TUNE_DAALA_DIST)),
      option("--lossless").doc("Enable lossless encoding").set(lossless, true),

      // pre-process
      option("--monochrome").doc("Encode to monochrome image.").set(codec.monochrome, 1u),
      option("--sharpness").doc("Sharpening output") & integer("0-7", sharpness),

      // post-process
      option("--disable-cdef").doc("Disable Constrained Directional Enhancement Filter").set(enableCDEF, false),
      option("--enable-cdef").doc("Enable Constrained Directional Enhancement Filter").set(enableCDEF, true),
      option("--disable-loop-restoration").doc("Disable Loop Restoration Filter").set(enableRestoration, false),
      option("--enable-loop-restoration").doc("Enable Loop Restoration Filter").set(enableRestoration, true),

      // coding parameter
      option("--tile-rows").doc("Number of tile rows") & integer("0-6", tileRows),
      option("--tile-colums").doc("Number of tile colums") & integer("0-6", tileColums),
      option("--disable-keyframe-temporal-filtering").doc("Disable temporal filtering on key frame").set(keyframeTemporalFiltering, false),
      option("--enable-keyframe-temporal-filtering").doc("Enable temporal filtering on key frame").set(keyframeTemporalFiltering, true),
      option("--disable-adaptive-quantization").doc("Disable adaptive quantization").set(adaptiveQuantization, false),
      option("--enable-adaptive-quantization").doc("Enable adaptive quantization").set(adaptiveQuantization, true),
      option("--superblock-size").doc("Superblock size.") & (parameter("dynamic").doc("encoder determines the size automatically.").set(superblockSize, AOM_SUPERBLOCK_SIZE_DYNAMIC) | parameter("128").doc("use 128x128 superblock.").set(superblockSize, AOM_SUPERBLOCK_SIZE_128X128) | parameter("64").doc("use 64x64 superblock.").set(superblockSize, AOM_SUPERBLOCK_SIZE_64X64))
  );
  if(!clipp::parse(argc, argv, cli)) {
    std::cerr << make_man_page(cli, basename(std::string(argv[0]))) << std::flush;
    return -1;
  }
  { // validate
    if(input == output) {
      std::cerr << make_man_page(cli, basename(std::string(argv[0]))) << std::flush;
      return -1;
    }
    if(!cropSize.has_value() && cropOffset.has_value()) {
      std::cerr << "crop-size must also be set, when crop-offset is set." << std::endl << std::flush;
      return -1;
    }
    if(cropSize.has_value() && !cropOffset.has_value()) {
      cropOffset = std::make_pair(std::make_pair(0,1), std::make_pair(0,1));
    }
  }
  return 0;
}

void Config::modify(aom_codec_ctx_t* aom) {
  //aom_codec_control(codec, AV1E_SET_DENOISE_NOISE_LEVEL, 1);

  // AOME_SET_ROI_MAP // FIXME: not implemented yet at libaom.
  // AOME_SET_ACTIVEMAP for internal use only
  // AOME_SET_SCALEMODE // FIXME(ledyba-z): it can be set, but not used.
  // AOME_SET_SPATIAL_LAYER_ID for adaptive video decoding (such as for Netflix or Youtube).
  aom_codec_control(aom, AOME_SET_CPUUSED, cpuUsed);
  aom_codec_control(aom, AOME_SET_SHARPNESS, sharpness);
  // AOME_SET_ENABLEAUTOALTREF is used only in 2nd pass(thus, is's for video).
  // AOME_SET_ENABLEAUTOBWDREF is for video (bwd-pred frames).
  // AOME_SET_STATIC_THRESHOLD // FIXME(ledyba-z): it can be set, but not used.
  // AOME_SET_ARNR_MAXFRAMES is for video.
  // AOME_SET_ARNR_STRENGTH is for video.
  aom_codec_control(aom, AOME_SET_TUNING, tune);
  aom_codec_control(aom, AOME_SET_CQ_LEVEL, crf);
  // It always can be 0(unlimited) for AVIF.
  aom_codec_control(aom, AOME_SET_MAX_INTRA_BITRATE_PCT, 0);
  // AOME_SET_NUMBER_SPATIAL_LAYERS for video
  // AV1E_SET_MAX_INTER_BITRATE_PCT for video
  // AV1E_SET_GF_CBR_BOOST_PCT for video.(I don't know what Golden Frame is)
  aom_codec_control(aom, AV1E_SET_LOSSLESS, lossless ? 1 : 0);
  aom_codec_control(aom, AV1E_SET_ROW_MT, rowMT ? 1 : 0);
  aom_codec_control(aom, AV1E_SET_TILE_ROWS, tileRows);
  aom_codec_control(aom, AV1E_SET_TILE_COLUMNS, tileColums);
  // AV1E_SET_ENABLE_TPL_MODEL is for video.
  aom_codec_control(aom, AV1E_SET_ENABLE_KEYFRAME_FILTERING, keyframeTemporalFiltering ? 1 : 0);
  // AV1E_SET_FRAME_PARALLEL_DECODING is for video. we have just one frame.
  // AV1E_SET_ERROR_RESILIENT_MODE is for video.
  // AV1E_SET_S_FRAME_MODE is for video.
  aom_codec_control(aom, AV1E_SET_AQ_MODE, adaptiveQuantization ? 1 : 0);
  // AV1E_SET_FRAME_PERIODIC_BOOST is for video.
  // AV1E_SET_NOISE_SENSITIVITY // FIXME(ledyba-z): it can be set, but not used.
  // AV1E_SET_TUNE_CONTENT // FIXME(ledyba-z): it can be set, but not used.
  // AV1E_SET_CDF_UPDATE_MODE is for video.

  //FIXME(ledyba-z): support color profile. PNG can contain gamma correction and color profile.
  // Gamma Correction and Precision Color (PNG: The Definitive Guide)
  // http://www.libpng.org/pub/png/book/chapter10.html
  aom_codec_control(aom, AV1E_SET_COLOR_PRIMARIES, 2 );
  aom_codec_control(aom, AV1E_SET_MATRIX_COEFFICIENTS, 2 );
  aom_codec_control(aom, AV1E_SET_TRANSFER_CHARACTERISTICS, 2);

  aom_codec_control(aom, AV1E_SET_CHROMA_SAMPLE_POSITION, 0); // see libavif-container

  // AV1E_SET_MIN_GF_INTERVAL for video
  aom_codec_control(aom, AV1E_SET_COLOR_RANGE, fullColorRange ? 1 : 0);
  // AV1E_SET_RENDER_SIZE should be the same as the output size. It's default.
  // AV1E_SET_TARGET_SEQ_LEVEL_IDX for video.
  aom_codec_control(aom, AV1E_SET_SUPERBLOCK_SIZE, superblockSize);
  // AOME_SET_ENABLEAUTOBWDREF is for video.

  aom_codec_control(aom, AV1E_SET_ENABLE_CDEF, enableCDEF ? 1 : 0);
  aom_codec_control(aom, AV1E_SET_ENABLE_RESTORATION, enableRestoration ? 1 : 0);
  // AV1E_SET_FORCE_VIDEO_MODE must be 0 (we encode still picture), that is default.

  // AV1E_SET_ENABLE_OBMC is for video, motion prediction.
  // OBMC is "Overlapped Block Motion Compensation"
  // https://jmvalin.ca/papers/AV1_tools.pdf

  // AV1E_SET_DISABLE_TRELLIS_QUANT is for video(motion estimation).
  // https://en.wikipedia.org/wiki/Trellis_quantization

  //

  //
}
