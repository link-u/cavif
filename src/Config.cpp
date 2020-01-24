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

std::pair<uint32_t, uint32_t> parseFraction(std::string const& str) {
  auto pos = str.find('/');
  std::string first = trim(str.substr(0, pos));
  std::string second = trim(str.substr(pos + 1));
  return std::make_pair(std::stoi(first), std::stoi(second));
}

std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>> parseFractionPair(std::string const& str) {
  auto pos = str.find(',');
  if(pos == std::string::npos) {
    throw std::runtime_error("Invalid fraction pair. Example: \"3/2, 10/7\"");
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
      option("--rotation").doc("Set rotation meta data(irot). Counter-clockwise.") & (parameter("0").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot0)) | parameter("90").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot180)) | parameter("180").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot180)) | parameter("270").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot270))),
      option("--mirror").doc("Set mirror meta data(imir).") & (parameter("vertical").set(mirrorAxis, std::make_optional(avif::ImageMirrorBox::Axis::Vertical)) | parameter("horizontal").set(mirrorAxis, std::make_optional(avif::ImageMirrorBox::Axis::Horizontal))),
      option("--crop-size").doc("Crop size(clap).") & value("widthN/widthD,heightN/heightD").call([&](std::string const& str){ cropSize = parseFractionPair(str); }),
      option("--crop-offset").doc("Crop offset(clap).") & value("horizOffN/horizOffD,vertOffN/vertOffD").call([&](std::string const& str){ cropOffset = parseFractionPair(str); }),
      // encoding
      option("--profile").doc("AV1 Profile(0=base, 1=high, 2=professional)") & integer("0=base, 1=high, 2=professional", aom.g_profile),
      option("--monochrome").doc("Encode to monochrome image.").set(codec.monochrome, 1u),
      option("--usage").doc("Encoder usage") & (parameter("good").doc("Good Quality mode").set(aom.g_usage, static_cast<unsigned int>(AOM_USAGE_GOOD_QUALITY)) | parameter("realtime").doc("Real time encoding mode.").set(aom.g_usage, static_cast<unsigned int>(AOM_USAGE_REALTIME))),
      option("--threads") & integer("Num of threads to use", aom.g_threads),
      option("--pix-fmt").doc("Pixel format of output image.") & (parameter("yuv420").set(pixFmt, AOM_IMG_FMT_I420) | parameter("yuv422").set(pixFmt, AOM_IMG_FMT_I422) | parameter("yuv444").set(pixFmt, AOM_IMG_FMT_I444)),
      option("--bit-depth").doc("Bit depth of output image.") & (parameter("8").set(aom.g_bit_depth, AOM_BITS_8) | parameter("10").set(aom.g_bit_depth, AOM_BITS_10) | parameter("12").set(aom.g_bit_depth, AOM_BITS_12)),
      option("--rate-control").doc("Rate control method") & (parameter("q").doc("Constant Quality").set(aom.rc_end_usage, AOM_Q) | parameter("cq").doc("Constrained Quality").set(aom.rc_end_usage, AOM_CQ)),
      option("--enable-large-scale-tile").doc("Use large scale tile mode.").set(aom.large_scale_tile, 1u),
      option("--disable-large-scale-tile").doc("Don't use large scale tile mode.").set(aom.large_scale_tile, 0u),
      option("--full-still-picture-header").doc("Force to output full picture header").set(aom.full_still_picture_hdr, 1u),
      option("--enable-full-color-range").doc("Use full YUV color range.").set(fullColorRange, true),
      option("--disable-full-color-range").doc("Use limited YUV color range.").set(fullColorRange, false),
      option("--crf").doc("CQ Level in CQ rate control mode") & integer("0-63", crf),
      option("--cpu-used").doc("Quality/Speed ratio modifier") & integer("0-8", cpuUsed),
      option("--enable-cdef").doc("Enable Constrained Directional Enhancement Filter").set(enableCDEF, true),
      option("--disable-cdef").doc("Disable Constrained Directional Enhancement Filter").set(enableCDEF, false),
      option("--enable-restoration").doc("Enable Loop Restoration Filter").set(enableRestoration, true),
      option("--disable-restoration").doc("Disable Loop Restoration Filter").set(enableRestoration, false),
      option("--superblock-size").doc("Superblock size.") & (parameter("dynamic").doc("encoder determines the size automatically.").set(superblockSize, AOM_SUPERBLOCK_SIZE_DYNAMIC) | parameter("128").doc("use 128x128 superblock.").set(superblockSize, AOM_SUPERBLOCK_SIZE_128X128) | parameter("64").doc("use 64x64 superblock.").set(superblockSize, AOM_SUPERBLOCK_SIZE_64X64)),
      option("--tune").doc("Quality metric to tune") & (parameter("ssim").doc("structural similarity").set(tune, AOM_TUNE_SSIM) | parameter("psnr").doc("peak signal-to-noise ratio").set(tune, AOM_TUNE_PSNR) | parameter("cdef-dist").doc("cdef-dist").set(tune, AOM_TUNE_CDEF_DIST) | parameter("daala-dist").doc("daala-dist").set(tune, AOM_TUNE_DAALA_DIST))
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
  }
  return 0;
}

void Config::modify(aom_codec_ctx_t* aom) {
  //aom_codec_control(codec, AV1E_SET_DENOISE_NOISE_LEVEL, 1);
  aom_codec_control(aom, AOME_SET_CPUUSED, this->cpuUsed);
  aom_codec_control(aom, AOME_SET_STATIC_THRESHOLD, 0);
  aom_codec_control(aom, AOME_SET_TUNING, tune);
  aom_codec_control(aom, AOME_SET_CQ_LEVEL, this->crf);
  aom_codec_control(aom, AV1E_SET_ENABLE_CDEF, enableCDEF ? 1 : 0);
  aom_codec_control(aom, AV1E_SET_ENABLE_RESTORATION, enableRestoration ? 1 : 0);

  //FIXME(ledyba-z): support color profile. PNG can contain gamma correction and color profile.
  // Gamma Correction and Precision Color (PNG: The Definitive Guide)
  // http://www.libpng.org/pub/png/book/chapter10.html
  aom_codec_control(aom, AV1E_SET_COLOR_PRIMARIES, 2 );
  aom_codec_control(aom, AV1E_SET_MATRIX_COEFFICIENTS, 2 );
  aom_codec_control(aom, AV1E_SET_TRANSFER_CHARACTERISTICS, 2);
  //
  aom_codec_control(aom, AV1E_SET_COLOR_RANGE, fullColorRange ? 1 : 0);
  aom_codec_control(aom, AV1E_SET_SUPERBLOCK_SIZE, superblockSize);
}
