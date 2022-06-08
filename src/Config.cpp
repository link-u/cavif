//
// Created by psi on 2020/01/14.
//

#include <iostream>

#include "Config.hpp"

#include "ext/ExternalPartitionModelFactoryContainer.hpp"
#include "ext/ExternalPartitionModel.hpp"
#include "ext/models/NonSplitPartitionModel.hpp"

namespace {

// FIXME(ledyba-z): remove this function when the C++20 comes.
bool endsWith(std::string const& target, std::string const& suffix) {
  if(target.size() < suffix.size()) {
    return false;
  }
  return target.substr(target.size()-suffix.size()) == suffix;
}

std::string basename(std::string const& path) {
  auto const pos = path.find_last_of('/');
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
  return str;
}

avif::math::Fraction parseFraction(std::string const& str) {
  auto const pos = str.find('/');
  if(pos == std::string::npos) {
    return avif::math::Fraction(std::stoi(trim(str)), 1);
  } else {
    std::string const first = trim(str.substr(0, pos));
    std::string const second = trim(str.substr(pos + 1));
    int const n = std::stoi(first);
    int const d = std::stoi(second);
    if(d == 0) {
      throw std::invalid_argument("denominator_ can't be 0.");
    }
    return avif::math::Fraction(static_cast<int32_t>(n), static_cast<int32_t>(d)).reduce();
  }
}

std::pair<avif::math::Fraction, avif::math::Fraction> parseFractionPair(std::string const& str) {
  auto pos = str.find(',');
  if(pos == std::string::npos) {
    throw std::invalid_argument(R"(Invalid fraction pair. Example: "30/4, 100/7", "100, 100/2" or "100, 100")");
  }
  std::string const first = trim(str.substr(0, pos));
  std::string const second = trim(str.substr(pos + 1));
  return std::make_pair(parseFraction(first), parseFraction(second));
}

template <typename T>
std::optional<T> parseEnumFromInt(std::string const& str) {
  return std::make_optional(static_cast<T>(std::stoi(str)));
}

}

Config::Config(int argc, char** argv)
:commandName(basename(std::string(argv[0])))
,argc(argc)
,argv(argv)
,commandLineFlags(createCommandLineFlags())
{
}

void Config::usage() {
  std::cerr << make_man_page(commandLineFlags, commandName);
  std::cerr << std::flush;
}

bool Config::parse() {
  return !clipp::parse(argc, argv, commandLineFlags).any_error();
}

clipp::group Config::createCommandLineFlags() {
  using namespace clipp;
  auto& aom = this->codec;

  // Support flags.
  auto support = (
      option("-h", "--help").doc("Show help and exit.").set(showHelp, true)
  );

  // input/output
  group io = (
      required("-i", "--input").doc("Filename to input") & value("input.png", input),
      required("-o", "--output").doc("Filename to output") & value("output.avif", output),
      option("--attach-alpha").doc("Attach alpha plane") & value("input-alpha.avif").call([&](std::string const& str){ alphaInput = str; }),
      option("--attach-depth").doc("Attach depth plane") & value("input-depth.avif").call([&](std::string const& str){ depthInput = str; }),
      option("--encode-target").doc("Encode target") & (parameter("image").set(encodeTarget, EncodeTarget::Image).doc("Encode image planes (default)") | parameter("alpha").set(encodeTarget, EncodeTarget::Alpha).doc("Encode an alpha plane")),
      option("--show-result").doc("Show encoding result").set(showResult, true)
  );

  // meta
  group meta = (
      option("--rotation").doc("Set rotation meta data(irot). Counter-clockwise.") & (parameter("0").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot0)) | parameter("90").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot90)) | parameter("180").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot180)) | parameter("270").set(rotation, std::make_optional(avif::ImageRotationBox::Rotation::Rot270))),
      option("--mirror").doc("Set mirror meta data(imir).") & (parameter("vertical").set(mirrorAxis, std::make_optional(avif::ImageMirrorBox::Axis::Vertical)) | parameter("horizontal").set(mirrorAxis, std::make_optional(avif::ImageMirrorBox::Axis::Horizontal))),
      option("--crop-size").doc("Set crop size.") & value("width,height").call([&](std::string const& str){ cropSize = parseFractionPair(str); }),
      option("--crop-offset").doc("Set crop offset.") & value("horizontalOffset,verticalOffset").call([&](std::string const& str){ cropOffset = parseFractionPair(str); })
  );
  // av1 sequence header
  auto av1 = (
      // av1 sequence header
      option("--full-still-picture-header").doc("Force to output full picture header").set(aom.full_still_picture_hdr, 1u)
  );

  // colors
  group color = (
      option("--color-primaries").doc("Set color primaries information value.") & (
          /* 0 = For future use by ITU-T | ISO/IEC */
          integer("Value defined in H.273").call([&](std::string const& str){ colorPrimaries = parseEnumFromInt<avif::img::color::ColorPrimaries>(str); }).doc("See https://www.itu.int/rec/T-REC-H.273-201612-I/en") |
          parameter("bt709").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(1u))).doc("Rec. ITU-R BT.709-6") |
          parameter("sRGB").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(1u))).doc("IEC 61966-2-1 sRGB or sYCC") |
          parameter("sYCC").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(1u))).doc("IEC 61966-2-1 sRGB or sYCC") |
          parameter("unspecified").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(2u))).doc("Image characteristics are unknown or are determined by the application.") |
          /* 3 = For future use by ITU-T | ISO/IEC */
          parameter("bt470m").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(4u))).doc("Rec. ITU-R BT.470-6 System M (historical)") |
          parameter("bt470bg").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(5u))).doc("Rec. ITU-R BT.470-6 System B, G (historical)") |
          parameter("bt601").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(5u))).doc("Rec. ITU-R BT.601-7 625") |
          parameter("ntsc").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(6u))).doc("Rec. ITU-R BT.1700-0 NTSC") |
          parameter("smpte240m").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(7u))).doc("SMPTE ST 240 (1999)") |
          parameter("generic-film").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(8u))).doc("Generic film (colour filters using Illuminant C)") |
          parameter("bt2020").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(9u))).doc("Rec. ITU-R BT.2020-2") |
          parameter("bt2100").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(9u))).doc("Rec. ITU-R BT.2100-0") |
          parameter("smpte428").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(10u))).doc("SMPTE ST 428-1") |
          parameter("xyz").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(10u))).doc("(CIE 1931 XYZ as in ISO 11664-1)") |
          parameter("smpte431").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(11u))).doc("SMPTE RP 431-2 (2011)") |
          parameter("smpte432").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(12u))).doc("SMPTE EG 432-1 (2010)") |
          parameter("22").set(colorPrimaries, std::make_optional(static_cast<avif::img::color::ColorPrimaries>(22u))).doc("No corresponding industry specification identified")
      ),
      option("--transfer-characteristics").doc("Set transfer characteristics information value.") & (
          integer("Value defined in H.273").call([&](std::string const& str){ transferCharacteristics = parseEnumFromInt<avif::img::color::TransferCharacteristics>(str); }).doc("See https://www.itu.int/rec/T-REC-H.273-201612-I/en") |
          parameter("bt709").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(1u))).doc("Rec. ITU-R BT.709-6") |
          parameter("unspecified").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(2u))).doc("Image characteristics are unknown or are determined by the application.") |
          /* 3 = For future use by ITU-T | ISO/IEC */
          parameter("bt470m").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(4u))).doc("Rec. ITU-R BT.470-6 System M (historical)") |
          parameter("bt470bg").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(5u))).doc("Rec. ITU-R BT.470-6 System B, G (historical)") |
          parameter("bt601").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(6u))).doc("Rec. ITU-R BT.1700-0 NTSC") |
          parameter("ntsc").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(6u))).doc("Rec. ITU-R BT.1700-0 NTSC") |
          parameter("smpte240m").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(7u))).doc("SMPTE 240M (1999) (historical)") |
          parameter("linear").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(8u))).doc("Linear transfer characteristics") |
          parameter("log100").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(9u))).doc("Logarithmic transfer characteristic (100:1 range)") |
          parameter("log100sqrt10").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(10u))).doc("Logarithmic transfer characteristic (100 * Sqrt( 10 ) : 1 range)") |
          parameter("iec61966").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(11u))).doc("IEC 61966-2-4") |
          parameter("bt1361").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(12u))).doc("Rec. ITU-R BT.1361-0 extended colour gamut system (historical)") |
          parameter("sRGB").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(13u))).doc("IEC 61966-2-1 sRGB or sYCC") |
          parameter("sYCC").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(13u))).doc("IEC 61966-2-1 sRGB or sYCC") |
          parameter("bt2020").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(14u))).doc("Rec. ITU-R BT.2020-2 (10-bit system)") |
          parameter("bt2020-10bit").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(14u))).doc("Rec. ITU-R BT.2020-2 (10-bit system)") |
          parameter("bt2020-12bit").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(15u))).doc("Rec. ITU-R BT.2020-2 (12-bit system)") |
          parameter("smpte2084").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(16u))).doc("SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems") |
          parameter("bt2100pq").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(16u))).doc("Rec. ITU-R BT.2100-0 perceptual quantization (PQ) system") |
          parameter("smpte428").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(17u))).doc("SMPTE ST 428-1") |
          parameter("bt2100hlg").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(18u))).doc("Rec. ITU-R BT.2100-0 hybrid log-gamma (HLG) system") |
          parameter("arib-b67").set(transferCharacteristics, std::make_optional(static_cast<avif::img::color::TransferCharacteristics>(18u))).doc("ARIB STD-B67")
      ),
      option("--matrix-coefficients").doc("Set matrix coefficients information value.") & (
          integer("Value defined in H.273").call([&](std::string const& str){ matrixCoefficients = parseEnumFromInt<avif::img::color::MatrixCoefficients>(str); }).doc("See https://www.itu.int/rec/T-REC-H.273-201612-I/en") |
          parameter("bt709").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(1u))).doc("Rec. ITU-R BT.709-6 (default)") |
          parameter("sRGB").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(1u))).doc("IEC 61966-2-1 sRGB or sYCC (default)") |
          parameter("unspecified").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(2u))).doc("Image characteristics are unknown or are determined by the application") |
          /* 3 = For future use by ITU-T | ISO/IEC */
          parameter("us-fcc").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(4u))).doc("United States Federal Communications Commission (2003)") |
          parameter("bt470bg").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(4u))).doc("Rec. ITU-R BT.470-6 System B, G (historical)") |
          parameter("sYCC").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(5u))).doc("IEC 61966-2-1 sRGB or sYCC") |
          parameter("bt601").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(5u))).doc("Rec. ITU-R BT.601-7 625") |
          parameter("ntsc").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(6u))).doc("Rec. ITU-R BT.1700-0 NTSC") |
          parameter("smpte240m").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(7u))).doc("SMPTE 240M") |
          parameter("bt2020").set(matrixCoefficients, std::make_optional(static_cast<avif::img::color::MatrixCoefficients>(9u))).doc("Rec. ITU-R BT.2020-2 (non-constant luminance)")
      )
  );

  auto scales = (
      option("--horizontal-scale-mode").doc("Set horizontal scale mode") & (parameter("1/1").set(scaleMode.h_scaling_mode, AOME_NORMAL).doc("Do not scale (default)") | parameter("1/2").set(scaleMode.h_scaling_mode, AOME_ONETWO).doc("Scale to 1/2") | parameter("3/5").set(scaleMode.h_scaling_mode, AOME_THREEFIVE).doc("Scale to 3/5") | parameter("4/5").set(scaleMode.h_scaling_mode, AOME_FOURFIVE).doc("Scale to 4/5") | parameter("1/4").set(scaleMode.h_scaling_mode, AOME_ONEFOUR).doc("Scale to 1/4") | parameter("3/4").set(scaleMode.h_scaling_mode, AOME_THREEFOUR).doc("Scale to 3/4") | parameter("1/8").set(scaleMode.h_scaling_mode, AOME_ONEEIGHT).doc("Scale to 1/8")),
      option("--vertical-scale-mode").doc("Set vertical scale mode")     & (parameter("1/1").set(scaleMode.v_scaling_mode, AOME_NORMAL).doc("Do not scale (default)") | parameter("1/2").set(scaleMode.v_scaling_mode, AOME_ONETWO).doc("Scale to 1/2") | parameter("3/5").set(scaleMode.v_scaling_mode, AOME_THREEFIVE).doc("Scale to 3/5") | parameter("4/5").set(scaleMode.v_scaling_mode, AOME_FOURFIVE).doc("Scale to 4/5") | parameter("1/4").set(scaleMode.v_scaling_mode, AOME_ONEFOUR).doc("Scale to 1/4") | parameter("3/4").set(scaleMode.v_scaling_mode, AOME_THREEFOUR).doc("Scale to 3/4") | parameter("1/8").set(scaleMode.v_scaling_mode, AOME_ONEEIGHT).doc("Scale to 1/8")),
      option("--resize-mode").doc("Set resize mode") & (parameter("none").set(codec.rc_resize_mode, (unsigned int)(RESIZE_NONE)).doc("Do not resize") | parameter("fixed").set(codec.rc_resize_mode, (unsigned int)(RESIZE_FIXED)).doc("Resize image using a denominator_ given by `--resize-denominator_` arg") | parameter("random").set(codec.rc_resize_mode, (unsigned int)(RESIZE_RANDOM)).doc("Resize image randomly!")),
      option("--resize-denominator_").doc("Set resize denominator_.") & (integer("[8-16], default=8", codec.rc_resize_kf_denominator)),
      option("--superres-mode").doc("Set superres mode") & (parameter("none").set(codec.rc_superres_mode, AOM_SUPERRES_NONE).doc("Do not use superres mode") | parameter("fixed").set(codec.rc_superres_mode, AOM_SUPERRES_FIXED).doc("Apply superres filter to image using a denominator_ given by `--superres-denominator_` arg") | parameter("random").set(codec.rc_superres_mode, AOM_SUPERRES_RANDOM).doc("Apply superres filter to image with a random denominator_!") | parameter("qthresh").set(codec.rc_superres_mode, AOM_SUPERRES_QTHRESH).doc("Apply or do not apply superres filter to image based on the q index") | parameter("auto").set(codec.rc_superres_mode, AOM_SUPERRES_AUTO).doc("Apply or do not apply superres filter to image automatically")),
      option("--superres-denominator_").doc("Set superres resize denominator_.") & (integer("[8-16], default=8", codec.rc_superres_kf_denominator)),
      option("--superres-qthresh").doc("Set q level threshold for superres.") & (integer("[0-63], default=63 (Do not apply superres filter)", codec.rc_superres_kf_qthresh)),
      option("--render-width").doc("Set render width explicitly") & (integer("<render-width>", renderWidth)),
      option("--render-height").doc("Set render height explicitly") & (integer("<render-height>", renderHeight))
  );

  // profile and pixel formats
  group pixelAndColor = (
      option("--profile").doc("AV1 Profile(0=base, 1=high, 2=professional)") & integer("0=base(default), 1=high, 2=professional", aom.g_profile),
      option("--pix-fmt").doc("Pixel format of output image") & (parameter("yuv420").set(pixFmt, AOM_IMG_FMT_I420).doc("YUV420 format (default)") | parameter("yuv422").set(pixFmt, AOM_IMG_FMT_I422).doc("YUV422 format") | parameter("yuv444").set(pixFmt, AOM_IMG_FMT_I444).doc("YUV444 format (recommended for lossless images)")),
      option("--bit-depth").doc("Bit depth of output image") & (parameter("8").set(aom.g_bit_depth, AOM_BITS_8).doc("8bits per color, 24bits per pixel (default)") | parameter("10").set(aom.g_bit_depth, AOM_BITS_10).doc("10bits per color, 30bits per pixel") | parameter("12").set(aom.g_bit_depth, AOM_BITS_12).doc("12bits per color, 36bits per pixel")),
      option("--disable-full-color-range").doc("Use limited YUV color range").set(fullColorRange, false),
      option("--enable-full-color-range").doc("Use full YUV color range (default)").set(fullColorRange, true)
  );

  // trade-offs between speed and quality.
  group multiThreading = (
      option("--encoder-usage").doc("Encoder usage") & (parameter("good").set(aom.g_usage, static_cast<unsigned int>(AOM_USAGE_GOOD_QUALITY)).doc("Good Quality mode (default)") | parameter("realtime").set(aom.g_usage, static_cast<unsigned int>(AOM_USAGE_REALTIME)).doc("Real time encoding mode")),
      option("--threads") & integer("Num of threads to use (default=num of logical cores)", aom.g_threads),
      option("--enable-row-mt").doc("Enable row based multi-threading of encoder").set(rowMT, true),
      option("--disable-row-mt").doc("Disable row based multi-threading of encoder (default)").set(rowMT, false),
      option("--cpu-used").doc("Quality/Speed ratio modifier") & integer("0-9, default=1. Higher means slower encoding and better quality", cpuUsed)
  );

  // rate-control
  group rateControl= (
      option("--rate-control").doc("Rate control method") & (parameter("cbr").set(aom.rc_end_usage, AOM_CBR).doc("Constant Bit Rate mode. Please also set `--bit-rate` arg.") | parameter("q").set(aom.rc_end_usage, AOM_Q).doc("Constant Quality (default)") | parameter("cq").set(aom.rc_end_usage, AOM_CQ).doc("Constrained Quality")),
      option("--bit-rate").doc("Bit rate of output image.") & integer("kilo-bits per frame(default=0)", aom.rc_target_bitrate),
      option("--crf").doc("CQ Level in CQ rate control mode") & integer("0-63(default=10)", crf),
      option("--qmin").doc("Minimum (Best Quality) Quantizer") & integer("0-63(default=0)", codec.rc_min_quantizer),
      option("--qmax").doc("Maximum (Worst Quality) Quantizer") & integer("0-63(default=63)", codec.rc_max_quantizer),
      option("--adaptive-quantization").doc("Set adaptive-quantization mode") & (parameter("none").doc("none(default)").set(adaptiveQuantizationMode, int(NO_AQ)) | parameter("variance").doc("variance based").set(adaptiveQuantizationMode, int(VARIANCE_AQ)) | parameter("complexity").doc("complexity based").set(adaptiveQuantizationMode, int(VARIANCE_AQ)) | parameter("cyclic").doc("Cyclic refresh").set(adaptiveQuantizationMode, int(CYCLIC_REFRESH_AQ))),
      option("--enable-adaptive-quantization-b").doc("use adaptive quantize_b").set(enableAdaptiveQuantizationB, true),
      option("--disable-adaptive-quantization-b").doc("use traditional adaptive quantization (default)").set(enableAdaptiveQuantizationB, false),
      option("--delta-q").doc("a mode of delta q mode feature, that allows modulating q per superblock") & (parameter("none").doc("disable deltaQ").set(deltaQMode, int(NO_DELTA_Q)) | parameter("objective").doc("Use modulation to maximize objective quality").set(deltaQMode, int(DELTA_Q_OBJECTIVE)) | parameter("perceptual").doc("Use modulation to maximize perceptual quality").set(deltaQMode, int(DELTA_Q_PERCEPTUAL))),
      option("--delta-q-strength").doc("strength of deltaQ [0..1000] (default = 100)").set(deltaQStrength),
      option("--enable-chroma-delta-q").doc("enable delta quantization in chroma").set(enableChromaDeltaQ, true),
      option("--disable-chroma-delta-q").doc("disable delta quantization in chroma").set(enableChromaDeltaQ, false),
      option("--enable-loop-filter").doc("enable loop filter (default)").set(enableLoopFilter, true),
      option("--disable-loop-filter").doc("disable loop filter").set(enableLoopFilter, false),
      option("--enable-delta-lf").doc("enable delta loop filter").set(enableDeltaLoopFilter, true),
      option("--disable-delta-lf").doc("disable delta loop filter (default)").set(enableDeltaLoopFilter, false),
      option("--use-qm").doc("Use QMatrix").set(useQM, true),
      option("--qm-min").doc("Min quant matrix flatness") & integer("0-15 (default: 5)", qmMin),
      option("--qm-max").doc("Max quant matrix flatness") & integer("0-15 (default: 9)", qmMax),
      option("--qm-min-y").doc("Min quant matrix flatness for Y") & integer("0-15 (default: 10)", qmMinY),
      option("--qm-min-u").doc("Min quant matrix flatness for U") & integer("0-15 (default: 11)", qmMinU),
      option("--qm-min-v").doc("Min quant matrix flatness for V") & integer("0-15 (default: 12)", qmMinV),
      // TODO(ledyba-z): Support butteraugli. Don't forget to set CONFIG_TUNE_BUTTERAUGLI in CMakeLists.txt
      option("--tune").doc("Quality metric to tune") & (parameter("ssim").doc("SSIM(structural similarity)").set(tune, AOM_TUNE_SSIM) | parameter("psnr").doc("PSNR(peak signal-to-noise ratio)").set(tune, AOM_TUNE_PSNR)  | parameter("vmaf-with-preprocessing").doc("vmaf-with-preprocessing").set(tune, AOM_TUNE_VMAF_WITH_PREPROCESSING) | parameter("vmaf-without-preprocessing").doc("vmaf-without-preprocessing").set(tune, AOM_TUNE_VMAF_WITHOUT_PREPROCESSING) | parameter("vmaf-max-gain").doc("vmaf-max-gain").set(tune, AOM_TUNE_VMAF_MAX_GAIN) | parameter("vmaf-neg-max-gain").doc("vmaf-neg-max-gain").set(tune, AOM_TUNE_VMAF_NEG_MAX_GAIN) /* | parameter("butteraugli").doc("google's butteraugli algorithm (github.com/google/butteraugli)").set(tune, AOM_TUNE_BUTTERAUGLI)*/),
      option("--content-type").doc("Content type") & (parameter("default").doc("Regular video content (default)").set(contentType, AOM_CONTENT_DEFAULT) | parameter("screen").doc("Screen capture content").set(contentType, AOM_CONTENT_SCREEN) | parameter("film").doc("Film content").set(contentType, AOM_CONTENT_FILM)),
      option("--vmaf-model-path").doc("VMAF model file path to tuning image quality") & value("<path-to-vmaf-model-file>", vmafModelPath),
      option("--lossless").doc("Enable lossless encoding").set(lossless, true)
  );

  // pre-process
  group preProcess = (
      option("--monochrome").doc("Encode to monochrome image").set(codec.monochrome, 1u),
      option("--sharpness").doc("Sharpening output") & integer("0-7", sharpness)
  );

  // post-process
  group postProcess = (
      option("--disable-cdef").doc("Disable Constrained Directional Enhancement Filter (default)").set(enableCDEF, false),
      option("--enable-cdef").doc("Enable Constrained Directional Enhancement Filter").set(enableCDEF, true),
      option("--disable-loop-restoration").doc("Disable Loop Restoration Filter (default)").set(enableRestoration, false),
      option("--enable-loop-restoration").doc("Enable Loop Restoration Filter").set(enableRestoration, true)
  );

  // coding parameter
  group codingParameters = (
      option("--superblock-size").doc("Superblock size.") & (parameter("dynamic").doc("encoder determines the size automatically.").set(superblockSize, AOM_SUPERBLOCK_SIZE_DYNAMIC) | parameter("128").doc("use 128x128 superblock.").set(superblockSize, AOM_SUPERBLOCK_SIZE_128X128) | parameter("64").doc("use 64x64 superblock.").set(superblockSize, AOM_SUPERBLOCK_SIZE_64X64)),
      option("--tile-rows").doc("Number of tile rows") & integer("0-6", tileRows),
      option("--tile-columns").doc("Number of tile columns") & integer("0-6", tileColumns),
      option("--keyframe-temporal-filter").doc("Enable temporal filtering on key frame") & (parameter("disable").set(keyframeTemporalFilter, 0) | parameter("without-overlay").set(keyframeTemporalFilter, 1) | parameter("with-overlay").set(keyframeTemporalFilter, 2)),
      option("--enable-rect-partitions").doc("enable rectangular partitions (default)").set(enableRectPartition, true),
      option("--disable-rect-partitions").doc("disable rectangular partitions").set(enableRectPartition, false),
      option("--enable-ab-partitions").doc("enable ab partitions (default)").set(enableABPartition, true),
      option("--disable-ab-partitions").doc("disable ab partitions").set(enableABPartition, false),
      option("--disable-1to4-partitions").doc("enable 1to4 partitions (default)").set(enable1to4Partition, true),
      option("--enable-1to4-partitions").doc("disable 1to4 partitions").set(enable1to4Partition, false),
      option("--enable-intra-edge-filter").doc("enable intra edge filter (default)").set(enableIntraEdgeFilter, true),
      option("--disable-intra-edge-filter").doc("disable intra edge filter").set(enableIntraEdgeFilter, false),
      option("--min-partition-size").doc("min partition size") & (parameter("4").set(minPartitionSize, 4) | parameter("8").set(minPartitionSize, 8) | parameter("16").set(minPartitionSize, 16) | parameter("32").set(minPartitionSize, 32) | parameter("64").set(minPartitionSize, 64) | parameter("128").set(minPartitionSize, 128)),
      option("--max-partition-size").doc("max partition size") & (parameter("4").set(maxPartitionSize, 4) | parameter("8").set(maxPartitionSize, 8) | parameter("16").set(maxPartitionSize, 16) | parameter("32").set(maxPartitionSize, 32) | parameter("64").set(maxPartitionSize, 64) | parameter("128").set(maxPartitionSize, 128)),
      option("--enable-tx64").doc("enable 64-length transforms (default)").set(enableTX64, true),
      option("--disable-tx64").doc("disable 64-length transforms").set(enableTX64, false),
      option("--enable-flip-idtx").doc("enable flip and identity transforms (default)").set(enableFlipIDTX, true),
      option("--disable-flip-idtx").doc("disable flip and identity transforms").set(enableFlipIDTX, false),
      option("--enable-rect-tx").doc("enable rectangular transforms (default)").set(enableRectTX, true),
      option("--disable-rect-tx").doc("disable rectangular transforms").set(enableRectTX, false),
      option("--use-dct-only").doc("Use DCT tx onlyq").set(useDCTOnly, true),
      option("--use-default-tx-only").doc("use default tx type only").set(useDefaultTXOnly, true),
      option("--use-reduced-tx-set").doc("use reduced tx set, transforms w/o flip (4) + Identity (1).").set(useReducedTXSet, true),
      option("--enable-filter-intra").doc("enable (default)").set(enableFilterIntra, true),
      option("--disable-filter-intra").doc("disable ").set(enableFilterIntra, false),
      option("--enable-smooth-intra").doc("enable (default)").set(enableSmoothIntra, true),
      option("--disable-smooth-intra").doc("disable ").set(enableSmoothIntra, false),
      option("--enable-paeth-intra").doc("enable (default)").set(enablePaethIntra, true),
      option("--disable-paeth-intra").doc("disable ").set(enablePaethIntra, false),
      option("--enable-chroma-from-luma").doc("enable (default)").set(enableChromaFromLuma, true),
      option("--disable-chroma-from-luma").doc("disable ").set(enableChromaFromLuma, false),
      option("--enable-superres").doc("enable frame superresolution (default)").set(enableSuperres, true),
      option("--disable-superres").doc("disable frame superresolution").set(enableSuperres, false),
      option("--enable-palette").doc("enable palette mode").set(enablePalette, true),
      option("--disable-palette").doc("disable palette mode (default)").set(enablePalette, false),
      option("--enable-intrabc").doc("enable intra block copy mode (default)").set(enableIntraBC, true),
      option("--disable-intrabc").doc("disable intra block copy mode").set(enableIntraBC, false),
      option("--enable-angle-delta").doc("enable intra angle delta (default)").set(enableAngleDelta, true),
      option("--disable-angle-delta").doc("disable intra angle delta").set(enableAngleDelta, false),
      option("--enable-diagonal-intra").doc("enable usage of D45 to D203 intra modes (default)").set(enableDiagonalIntra, true),
      option("--disable-diagonal-intra").doc("disable usage of D45 to D203 intra modes").set(enableDiagonalIntra, false),
      option("--enable-directional-intra").doc("turn on directional intra mode (default)").set(enableDiagonalIntra, true),
      option("--disable-directional-intra").doc("turn off directional intra mode").set(enableDiagonalIntra, false),
      option("--enable-tx-size-search").doc("turn on transform size search (default). Transforms always have the largest possible size").set(enableTxSizeSearch, true),
      option("--disable-tx-size-search").doc("turn off transform size search. Search for the best transform size for each block").set(enableTxSizeSearch, false)
  );

  //auto partitioning = (
  //);

  return (io, meta, av1, color, scales, pixelAndColor, multiThreading, rateControl, preProcess, postProcess, codingParameters) | support;
}

void Config::validateBeforeLoad() const {
  if(input == output) {
    throw std::invalid_argument("Input and output can't be the same file!");
  }
  if(!endsWith(input, ".png")) {
    throw std::invalid_argument("please give png file for input");
  }
  if(!endsWith(output, ".avif")) {
    throw std::invalid_argument("please give avif file for output");
  }
  if(
      (colorPrimaries.has_value() || transferCharacteristics.has_value() || matrixCoefficients.has_value()) &&
      !(colorPrimaries.has_value() && transferCharacteristics.has_value() && matrixCoefficients.has_value())
  ) {
    throw std::invalid_argument("All of (or none of) --color-primaries, --transfer-characteristics and --matrix-coefficients should be set.");
  }
  if(
      (cropSize.has_value() || cropOffset.has_value()) &&
      !(cropSize.has_value() && cropOffset.has_value())
  ) {
    throw std::invalid_argument("both crop-size and crop-offset must be set.");
  }

}

void Config::validateAfterLoad(const uint32_t width, const uint32_t height) const {
  /*
ISO/IEC 23000-22:2019/Amd. 2:2021(E)

7.3.6.7
Replace the text with the following:
The clean aperture (cropping) property may be associated with any image and shall be supported by
the MIAF reader. The clean aperture property is restricted according to the chroma sampling format of
the input image (4:4:4, 4:2:2:, 4:2:0, or 4:0:0) as follows:
— cleanApertureWidth and cleanApertureHeight shall be integers;
— The leftmost pixel and the topmost line of the clean aperture as defined in ISO/IEC 14496-12:2020,
Section 12.1.4.1 shall be integers;
— If chroma is subsampled horizontally (i.e., 4:2:2 and 4:2:0), the leftmost pixel of the clean aperture
shall be even numbers;
— If chroma is subsampled vertically (i.e., 4:2:0), the topmost line of the clean aperture shall be even
numbers.
 */
  if(cropSize.has_value() && cropOffset.has_value()) {
    using avif::math::Fraction;
    auto const [cropWidth, cropHeight] = this->cropSize.value();
    auto const [offX, offY] = this->cropOffset.value();
    if (!(cropWidth.isInteger() && cropHeight.isInteger())) {
      throw std::invalid_argument("crop size must be integers.");
    }
    // ISO/IEC 14496-12:2015(E)
    // p.157
    auto const left =
        Fraction(static_cast<int32_t>(width - 1),2).reduce()
        .add(offX)
        .minus(cropWidth.minus(Fraction(1,1)).div(2));
    auto const top =
        Fraction(static_cast<int32_t>(height - 1),2).reduce()
        .add(offY)
        .minus(cropHeight.minus(Fraction(1,1)).div(2));
    if(!left.isInteger()) {
      throw std::invalid_argument("The leftmost pixel must be an integer.");
    }
    if(!top.isInteger()) {
      throw std::invalid_argument("The topmost pixel must be an integer.");
    }
    if(pixFmt == AOM_IMG_FMT_I420 || pixFmt == AOM_IMG_FMT_I422) {
      if(left.numerator() % 2 != 0) {
        throw std::invalid_argument("The leftmost pixel must be an even.");
      }
    }
    if(pixFmt == AOM_IMG_FMT_I420) {
      if(top.numerator() % 2 != 0) {
        throw std::invalid_argument("The topmost pixel must be an even.");
      }
    }
  }
}

std::optional<avif::img::ColorProfile> Config::calcColorProfile() const {
  avif::img::ColorProfile profile;
  profile.cicp = std::make_optional<avif::ColourInformationBox::CICP>();
  if(colorPrimaries.has_value() && transferCharacteristics.has_value() && matrixCoefficients.has_value()) {
    profile.cicp->colourPrimaries = static_cast<uint16_t>(colorPrimaries.value());
    profile.cicp->transferCharacteristics = static_cast<uint16_t>(transferCharacteristics.value());
    profile.cicp->colourPrimaries = static_cast<uint16_t>(colorPrimaries.value());
  }
  profile.cicp->fullRangeFlag = fullColorRange;
  return profile;
}

void Config::modify(aom_codec_ctx_t* aom, avif::img::ColorProfile const& colorProfile) {
  // MEMO(ledyba-z): These qp offset parameters are only used in video.
  //codec.use_fixed_qp_offsets = 1;
  //codec.fixed_qp_offsets[0] = 0;

  #define set(param, expr) \
    if(aom_codec_control(aom, param, (expr)) != AOM_CODEC_OK) { \
      throw std::invalid_argument(std::string("Failed to set [" #param "] : ") + aom_codec_error_detail(aom)); \
    }

  (void)AOME_USE_REFERENCE; // It's for movies.(Anyway, it's not used in current libaom)

  //FIXME: not implemented yet at libaom.
  // It can be useful for manga images.
  //aom_codec_control(codec, AOME_SET_ROI_MAP, ...);
  (void)AOME_SET_ROI_MAP;

  (void)AOME_SET_ACTIVEMAP; // FIXME(ledyba-z): Not used currently.
  if(!(scaleMode.h_scaling_mode == AOME_NORMAL && scaleMode.v_scaling_mode == AOME_NORMAL)) {
    set(AOME_SET_SCALEMODE, &scaleMode);
  }
  (void)AOME_SET_SPATIAL_LAYER_ID; // for adaptive video decoding (such as for Netflix or Youtube).
  set(AOME_SET_CPUUSED, cpuUsed);
  set(AOME_SET_SHARPNESS, sharpness);
  (void)AOME_SET_ENABLEAUTOALTREF; // is used only in 2nd pass(thus, it's for video).
  (void)AOME_SET_ENABLEAUTOBWDREF; // is for video (bwd-pred frames).
  (void)AOME_SET_STATIC_THRESHOLD; // TODO: Not used. I don't know it's for key-frame or not.
  (void)AOME_SET_ARNR_MAXFRAMES; // is for video.
  (void)AOME_SET_ARNR_STRENGTH; // is for video.
  set(AOME_SET_TUNING, tune);
  set(AOME_SET_CQ_LEVEL, crf);
  // It always can be 0(unlimited) for AVIF.
  set(AOME_SET_MAX_INTRA_BITRATE_PCT, 0);
  (void)AOME_SET_NUMBER_SPATIAL_LAYERS; // for video
  (void)AV1E_SET_MAX_INTER_BITRATE_PCT; // for video
  (void)AV1E_SET_GF_CBR_BOOST_PCT; // for video.(I don't know what Golden Frame actually means)
  set(AV1E_SET_LOSSLESS, lossless ? 1 : 0);
  set(AV1E_SET_ROW_MT, rowMT ? 1 : 0);
  set(AV1E_SET_TILE_ROWS, tileRows);
  set(AV1E_SET_TILE_COLUMNS, tileColumns);
  (void)AV1E_SET_ENABLE_TPL_MODEL; // is for video.
  set(AV1E_SET_ENABLE_KEYFRAME_FILTERING, keyframeTemporalFilter);
  (void)AV1E_SET_FRAME_PARALLEL_DECODING; // is for video. we have just one frame. Finally, we just encode.
  (void)AV1E_SET_ERROR_RESILIENT_MODE; // is for video.
  (void)AV1E_SET_S_FRAME_MODE; // is for video.
  set(AV1E_SET_AQ_MODE, adaptiveQuantizationMode);
  (void)AV1E_SET_FRAME_PERIODIC_BOOST; // is for video.

  //FIXME(ledyba-z): it can be set, but not used.
  // To check, `grep -R 'oxcf->noise_sensitivity' external/libaom/av1`
  // (updated at 2021/11: it is used, but guarded by OUTPUT_YUV_DENOISED, which is not documented yet.
  // set(AV1E_SET_NOISE_SENSITIVITY, 0);

  set(AV1E_SET_TUNE_CONTENT, contentType);

  (void)AV1E_SET_CDF_UPDATE_MODE; // is for video.

  set(AV1E_SET_COLOR_PRIMARIES, colorProfile.cicp->colourPrimaries);
  set(AV1E_SET_TRANSFER_CHARACTERISTICS, colorProfile.cicp->transferCharacteristics);
  set(AV1E_SET_MATRIX_COEFFICIENTS, colorProfile.cicp->matrixCoefficients);

 // FIXME(ledyba-z): It's not used. see libavif-container to see our choice.
  (void)AV1E_SET_CHROMA_SAMPLE_POSITION;

  (void)AV1E_SET_MIN_GF_INTERVAL; // for video
  set(AV1E_SET_COLOR_RANGE, colorProfile.cicp->fullRangeFlag ? 1 : 0);
  (void)AV1E_SET_RENDER_SIZE; // should be the same as the output size. It's default.
  if(renderWidth > 0 && renderHeight > 0) {
    int renderSize[2] = {renderWidth, renderHeight};
    set(AV1E_SET_RENDER_SIZE, renderSize);
  }
  (void)AV1E_SET_TARGET_SEQ_LEVEL_IDX; // for video.
  (void)AV1E_GET_SEQ_LEVEL_IDX; // for video.
  set(AV1E_SET_SUPERBLOCK_SIZE, superblockSize);
  (void)AOME_SET_ENABLEAUTOBWDREF; // is for video.

  set(AV1E_SET_ENABLE_CDEF, enableCDEF ? 1 : 0);
  set(AV1E_SET_ENABLE_RESTORATION, enableRestoration ? 1 : 0);

  // we are working on images.
  set(AV1E_SET_FORCE_VIDEO_MODE, 0);

  (void)AV1E_SET_ENABLE_OBMC; // is for video, motion prediction.
  // OBMC means "Overlapped Block Motion Compensation"
  // https://jmvalin.ca/papers/AV1_tools.pdf

  (void)AV1E_SET_DISABLE_TRELLIS_QUANT; // is for video(motion estimation).
  // https://en.wikipedia.org/wiki/Trellis_quantization

  if(useQM) {
    set(AV1E_SET_ENABLE_QM, 1);
    set(AV1E_SET_QM_MIN, qmMin);
    set(AV1E_SET_QM_MAX, qmMax);
    set(AV1E_SET_QM_Y, qmMinY);
    set(AV1E_SET_QM_U, qmMinU);
    set(AV1E_SET_QM_V, qmMinV);
  }

  set(AV1E_SET_NUM_TG, (1u << static_cast<unsigned int>(tileRows)) + (1u << static_cast<unsigned int>(tileColumns)));

  (void)AV1E_SET_MTU; // is not needed to set, because AV1E_SET_NUM_TG is already set.
  set(AV1E_SET_ENABLE_RECT_PARTITIONS, enableRectPartition ? 1 : 0);
  set(AV1E_SET_ENABLE_AB_PARTITIONS, enableABPartition ? 1 : 0);
  set(AV1E_SET_ENABLE_1TO4_PARTITIONS, enable1to4Partition ? 1 : 0);
  set(AV1E_SET_MIN_PARTITION_SIZE, minPartitionSize);
  set(AV1E_SET_MAX_PARTITION_SIZE, maxPartitionSize);
  set(AV1E_SET_ENABLE_INTRA_EDGE_FILTER, enableIntraEdgeFilter ? 1 : 0);
  (void)AV1E_SET_ENABLE_ORDER_HINT; // is for video
  set(AV1E_SET_ENABLE_TX64, enableTX64 ? 1 : 0);
  set(AV1E_SET_ENABLE_FLIP_IDTX, enableFlipIDTX ? 1 : 0);
  set(AV1E_SET_ENABLE_RECT_TX, enableRectTX ? 1 : 0);
  (void)AV1E_SET_ENABLE_DIST_WTD_COMP; // is for video
  (void)AV1E_SET_ENABLE_REF_FRAME_MVS; // is for video
  (void)AV1E_SET_ALLOW_REF_FRAME_MVS; // is for video
  set(AV1E_SET_ENABLE_DUAL_FILTER, 0);
  set(AV1E_SET_ENABLE_CHROMA_DELTAQ, enableChromaDeltaQ ? 1 : 0);
  (void)AV1E_SET_ENABLE_MASKED_COMP; // is for video
  (void)AV1E_SET_ENABLE_ONESIDED_COMP; // is for video
  (void)AV1E_SET_ENABLE_INTERINTRA_COMP; // is for video
  (void)AV1E_SET_ENABLE_SMOOTH_INTERINTRA; // is for video
  (void)AV1E_SET_ENABLE_DIFF_WTD_COMP; // is for video
  (void)AV1E_SET_ENABLE_INTERINTER_WEDGE; // is for video
  (void)AV1E_SET_ENABLE_GLOBAL_MOTION; // is for video
  set(AV1E_SET_ENABLE_GLOBAL_MOTION, 0); // we are working on images.
  set(AV1E_SET_ENABLE_WARPED_MOTION, 0); // we are working on images.
  set(AV1E_SET_ALLOW_WARPED_MOTION, 0); // we are working on images.
  set(AV1E_SET_ENABLE_FILTER_INTRA, enableFilterIntra ? 1 : 0);
  set(AV1E_SET_ENABLE_SMOOTH_INTRA, enableSmoothIntra ? 1 : 0);
  set(AV1E_SET_ENABLE_PAETH_INTRA, enablePaethIntra ? 1 : 0);
  set(AV1E_SET_ENABLE_CFL_INTRA, enableChromaFromLuma ? 1 : 0);
  set(AV1E_SET_ENABLE_SUPERRES, enableSuperres ? 1 : 0);
  (void)AV1E_SET_ENABLE_OVERLAY; // is for video.
  set(AV1E_SET_ENABLE_PALETTE, enablePalette ? 1 : 0);
  set(AV1E_SET_ENABLE_INTRABC, enableIntraBC ? 1 : 0);
  set(AV1E_SET_DELTAQ_MODE, deltaQMode);
  set(AV1E_SET_DELTALF_MODE, enableDeltaLoopFilter ? 1 : 0);
  (void)AV1E_SET_SINGLE_TILE_DECODING; // We are working on encoding.
  (void)AV1E_ENABLE_MOTION_VECTOR_UNIT_TEST; // is for video.
  (void)AV1E_SET_TIMING_INFO_TYPE; // is for video.
  (void)AV1E_SET_FILM_GRAIN_TEST_VECTOR; // is for testing
  (void)AV1E_SET_FILM_GRAIN_TABLE; // can be supported, but it is mainly for video.
  (void)AV1E_SET_DENOISE_NOISE_LEVEL; // can be supported, but it is mainly for video.
  (void)AV1E_SET_DENOISE_BLOCK_SIZE; // can be supported, but it is mainly for video.
  set(AV1E_SET_REDUCED_TX_TYPE_SET, useReducedTXSet ? 1 : 0);
  set(AV1E_SET_INTRA_DCT_ONLY, useDCTOnly ? 1 : 0);
  set(AV1E_SET_INTRA_DEFAULT_TX_ONLY, useDefaultTXOnly ? 1 : 0);
  set(AV1E_SET_QUANT_B_ADAPT, enableAdaptiveQuantizationB ? 1 : 0);
  (void)AV1E_SET_GF_MAX_PYRAMID_HEIGHT; // is for video.
  (void)AV1E_SET_MAX_REFERENCE_FRAMES; // is for video.
  (void)AV1E_SET_REDUCED_REFERENCE_SET; // is for video.
  (void)AV1E_SET_COEFF_COST_UPD_FREQ; // may be for video, because it mentions tile decoding.
  (void)AV1E_SET_MODE_COST_UPD_FREQ; // may be for video, because it mentions tile decoding.
  (void)AV1E_SET_MV_COST_UPD_FREQ; // may be for video, because it mentions tile decoding.
  (void)AV1E_SET_TIER_MASK; // for video, because still picture always has 1 control point.
  (void)AV1E_SET_MIN_CR; // is for video. My GDB says that.
  (void)AV1E_SET_SVC_LAYER_ID; // is for video.
  (void)AV1E_SET_SVC_PARAMS; // is for video.
  (void)AV1E_SET_SVC_REF_FRAME_CONFIG; // is for video.
  if(!vmafModelPath.empty()) {
    set(AV1E_SET_VMAF_MODEL_PATH, vmafModelPath.c_str());
  }
  (void)AV1E_ENABLE_EXT_TILE_DEBUG;// is for debugging.
  (void)AV1E_ENABLE_SB_MULTIPASS_UNIT_TEST;// is for unit test.
  (void)AV1E_SET_GF_MIN_PYRAMID_HEIGHT; // is for video.

  (void)AV1E_SET_VBR_CORPUS_COMPLEXITY_LAP; // is for video.
  (void)AV1E_GET_BASELINE_GF_INTERVAL; // is for video.

  (void)AV1E_SET_ENABLE_DNL_DENOISING; // can be supported, but it is mainly for video (used in two pass mode).

  set(AV1E_SET_ENABLE_DIAGONAL_INTRA, enableDiagonalIntra ? 1 : 0);

  (void)AV1E_SET_DV_COST_UPD_FREQ; // is for intrabc motion vectors, for video.

  (void)AV1E_SET_PARTITION_INFO_PATH; // TODO(ledyba-z): It seems it's not recorded for keyframes. Read more papers!

  if (false) { // FIXME(ledyba-z): Can be configured, but not invoked at all. Read more papers!
    ExternalPartitionModelFactoryContainer container(std::make_unique<NonSplitPartitionModelFactory>());
    auto bridge = container.makeBridge();
    set(AV1E_SET_EXTERNAL_PARTITION, &bridge);
  }

  set(AV1E_SET_ENABLE_DIRECTIONAL_INTRA, enableDirectionalIntra ? 1 : 0);
  set(AV1E_SET_ENABLE_TX_SIZE_SEARCH, enableTxSizeSearch ? 1 : 0);

  (void)AV1E_SET_SVC_REF_FRAME_COMP_PRED; // is for video.

  set(AV1E_SET_DELTAQ_STRENGTH, deltaQStrength);
  set(AV1E_SET_LOOPFILTER_CONTROL, enableLoopFilter ? LOOPFILTER_ALL : LOOPFILTER_NONE);

  #undef set
}
