# cavif

|         | Status |
|--|---|
| Linux | [![Build on Linux](https://github.com/link-u/cavif/workflows/Build%20on%20Linux/badge.svg) ](https://github.com/link-u/cavif/actions?query=workflow%3A%22Build+on+Linux%22) |
| Linux(.deb) | [![Build debian packages](https://github.com/link-u/cavif/workflows/Build%20debian%20packages/badge.svg)](https://github.com/link-u/cavif/actions?query=workflow%3A%22Build+debian+packages%22) |
| macOS | [![Build on macOS](https://github.com/link-u/cavif/workflows/Build%20on%20macOS/badge.svg)](https://github.com/link-u/cavif/actions?query=workflow%3A%22Build+on+macOS%22) |
| Windows | [![Build on Windows](https://github.com/link-u/cavif/workflows/Build%20on%20Windows/badge.svg)](https://github.com/link-u/cavif/actions?query=workflow%3A%22Build+on+Windows%22) |

## Description (en)

avif encoder, using [libaom](https://aomedia.googlesource.com/aom/) directly.

[avif (AV1 Image File Format)](https://aomediacodec.github.io/av1-avif/) is a still picture format uses a keyframe of [AV1](https://aomediacodec.github.io/av1-spec/av1-spec.pdf).

The most significant mission of this project is "**Make it enable to tune libaom's all encoding options to optimize quality/size ratio as nice as possible human beings can**".

## Description (ja)

[AVIF(AV1 Image File Format)]((https://aomediacodec.github.io/av1-avif/))は、動画フォーマットである[AV1](https://aomediacodec.github.io/av1-spec/av1-spec.pdf)のキーフレームを流用して圧縮する静止画フォーマットです。

cavifは、ラッパーを介さず [libaom](https://aomedia.googlesource.com/aom/) を直接叩くavifのエンコーディング・コマンドです。このプロジェクトの唯一にして最大の使命は、 **libaomの静止画に関する全エンコードオプションを仔細に操作できるようにして、現生人類が現時点で達成可能な最高の圧縮効率を実現する手段を提供すること** です。

[日本語の詳しいドキュメントはこちら](./doc/ja_JP/README.md)。

# How to build

## Pre-requirements

### cmake >= 3.13

If your system cmake is lower than 3.13, please install the latest version:

To install:

- Ubuntu/Debian
  - See https://apt.kitware.com/
- Windows
  - [Download | CMake](https://cmake.org/download/)
  - [mingw-w64-cmake - MSYS2 Packages](https://packages.msys2.org/base/mingw-w64-cmake)
- mac OS
  - `brew install cmake`

### latest version of meson and ninja

meson and ninja are required to build libvmaf.

Please see: https://mesonbuild.com/Quick-guide.html

Or another method: You can use python's venv to install. [See our CI about details](https://github.com/link-u/cavif/blob/master/.github/workflows/build-on-linux.yml).

## Build steps

```bash
# cloning this repository with dependencies.
git clone --recurse-submodules --recursive https://github.com/link-u/cavif

cd cavif

# Apply workarounds
bash scripts/apply-patches.sh

# Build dependencies not managed by CMake.
bash scripts/build-deps.sh

# Make build directory
mkdir build && cd build

# If your system gcc is 8.0 or higher:
cmake -G 'Ninja' ..

# If not, please install gcc-8 (or higher) and tell them to CMake.
CXX=g++-8 CC=gcc-8 cmake -G 'Ninja' ..

# build and get cavif binary!
ninja
```

# Usage

## basic usage

```bash
cavif -i <input.png> -o <output.avif>
```

Example avif files are available in [AOMediaCodec/av1-avif](https://github.com/AOMediaCodec/av1-avif/tree/master/testFiles) or [link-u/avif-sample-images](https://github.com/link-u/avif-sample-images).

(Currently, detailed documentation is only in [Japanese](./doc/ja_JP/README.md))

## basic usage (alpha)

You have to encode alpha plane separately(Of course, it's to achieve best image quality!).

```bash
cavif -i <input.png> -o <output-alpha.avif> --encode-target alpha --monochrome
cavif -i <input.png> -o <output.avif> --encode-target image --attach-alpha <output-alpha.avif>
```

## SYNOPSIS

```
[2022/03/26 11:57:21 INFO ] cavif
[2022/03/26 11:57:21 INFO ] libaom ver: 3.3.0
[2022/03/26 11:57:21 INFO ] libpng ver:1.6.38.git
SYNOPSIS
        cavif -i <input.png> -o <output.avif> [--attach-alpha <input-alpha.avif>] [--attach-depth
              <input-depth.avif>] [--encode-target [image|alpha]] [--show-result] [--rotation
              [0|90|180|270]] [--mirror [vertical|horizontal]] [--crop-size <width,height>]
              [--crop-offset <horizontalOffset,verticalOffset>] [--full-still-picture-header]
              [--color-primaries (<Value defined in
              H.273>|bt709|sRGB|sYCC|unspecified|bt470m|bt470bg|bt601|ntsc|smpte240m|generic-film|bt2020|bt2100|smpte428|xyz|smpte431|smpte432|22)]
              [--transfer-characteristics (<Value defined in
              H.273>|bt709|unspecified|bt470m|bt470bg|bt601|ntsc|smpte240m|linear|log100|log100sqrt10|iec61966|bt1361|sRGB|sYCC|bt2020|bt2020-10bit|bt2020-12bit|smpte2084|bt2100pq|smpte428|bt2100hlg|arib-b67)]
              [--matrix-coefficients (<Value defined in
              H.273>|bt709|sRGB|unspecified|us-fcc|bt470bg|sYCC|bt601|ntsc|smpte240m|bt2020)]
              [--horizontal-scale-mode [1/1|1/2|3/5|4/5|1/4|3/4|1/8]] [--vertical-scale-mode
              [1/1|1/2|3/5|4/5|1/4|3/4|1/8]] [--resize-mode [none|fixed|random]]
              [--resize-denominator_ <[8-16], default=8>] [--superres-mode
              [none|fixed|random|qthresh|auto]] [--superres-denominator_ <[8-16], default=8>]
              [--superres-qthresh <[0-63], default=63 (Do not apply superres filter)>]
              [--render-width <<render-width>>] [--render-height <<render-height>>] [--profile
              <0=base(default), 1=high, 2=professional>] [--pix-fmt [yuv420|yuv422|yuv444]]
              [--bit-depth [8|10|12]] [--disable-full-color-range] [--enable-full-color-range]
              [--encoder-usage [good|realtime]] [--threads <Num of threads to use (default=num of
              logical cores)>] [--enable-row-mt] [--disable-row-mt] [--cpu-used <0-9, default=1.
              Higher means slower encoding and better quality>] [--rate-control [cbr|q|cq]]
              [--bit-rate <kilo-bits per frame(default=0)>] [--crf <0-63(default=10)>] [--qmin
              <0-63(default=0)>] [--qmax <0-63(default=63)>] [--adaptive-quantization
              [none|variance|complexity|cyclic]] [--enable-adaptive-quantization-b]
              [--disable-adaptive-quantization-b] [--delta-q [none|objective|perceptual]]
              [--delta-q-strength] [--enable-chroma-delta-q] [--disable-chroma-delta-q]
              [--enable-loop-filter] [--disable-loop-filter] [--enable-delta-lf]
              [--disable-delta-lf] [--use-qm] [--qm-min <0-15 (default: 5)>] [--qm-max <0-15
              (default: 9)>] [--qm-min-y <0-15 (default: 10)>] [--qm-min-u <0-15 (default: 11)>]
              [--qm-min-v <0-15 (default: 12)>] [--tune
              [ssim|psnr|vmaf-with-preprocessing|vmaf-without-preprocessing|vmaf-max-gain|vmaf-neg-max-gain]]
              [--content-type [default|screen|film]] [--vmaf-model-path <<path-to-vmaf-model-file>>]
              [--lossless] [--monochrome] [--sharpness <0-7>] [--disable-cdef] [--enable-cdef]
              [--disable-loop-restoration] [--enable-loop-restoration] [--superblock-size
              [dynamic|128|64]] [--tile-rows <0-6>] [--tile-columns <0-6>]
              [--keyframe-temporal-filter [disable|without-overlay|with-overlay]]
              [--enable-rect-partitions] [--disable-rect-partitions] [--enable-ab-partitions]
              [--disable-ab-partitions] [--disable-1to4-partitions] [--enable-1to4-partitions]
              [--enable-intra-edge-filter] [--disable-intra-edge-filter] [--min-partition-size
              [4|8|16|32|64|128]] [--max-partition-size [4|8|16|32|64|128]] [--enable-tx64]
              [--disable-tx64] [--enable-flip-idtx] [--disable-flip-idtx] [--enable-rect-tx]
              [--disable-rect-tx] [--use-dct-only] [--use-default-tx-only] [--use-reduced-tx-set]
              [--enable-filter-intra] [--disable-filter-intra] [--enable-smooth-intra]
              [--disable-smooth-intra] [--enable-paeth-intra] [--disable-paeth-intra]
              [--enable-chroma-from-luma] [--disable-chroma-from-luma] [--enable-superres]
              [--disable-superres] [--enable-palette] [--disable-palette] [--enable-intrabc]
              [--disable-intrabc] [--enable-angle-delta] [--disable-angle-delta]
              [--enable-diagonal-intra] [--disable-diagonal-intra] [--enable-directional-intra]
              [--disable-directional-intra] [--enable-tx-size-search] [--disable-tx-size-search]

        cavif -h

OPTIONS
        -i, --input Filename to input
        -o, --output
                    Filename to output

        --attach-alpha
                    Attach alpha plane

        --attach-depth
                    Attach depth plane

        --encode-target
                    Encode target

        image       Encode image planes (default)
        alpha       Encode an alpha plane
        --show-result
                    Show encoding result

        --rotation  Set rotation meta data(irot). Counter-clockwise.
        --mirror    Set mirror meta data(imir).
        --crop-size Set crop size.
        --crop-offset
                    Set crop offset.

        --full-still-picture-header
                    Force to output full picture header

        --color-primaries
                    Set color primaries information value.

        <Value defined in H.273>
                    See https://www.itu.int/rec/T-REC-H.273-201612-I/en

        bt709       Rec. ITU-R BT.709-6
        sRGB        IEC 61966-2-1 sRGB or sYCC
        sYCC        IEC 61966-2-1 sRGB or sYCC
        unspecified Image characteristics are unknown or are determined by the application.
        bt470m      Rec. ITU-R BT.470-6 System M (historical)
        bt470bg     Rec. ITU-R BT.470-6 System B, G (historical)
        bt601       Rec. ITU-R BT.601-7 625
        ntsc        Rec. ITU-R BT.1700-0 NTSC
        smpte240m   SMPTE ST 240 (1999)
        generic-film
                    Generic film (colour filters using Illuminant C)

        bt2020      Rec. ITU-R BT.2020-2
        bt2100      Rec. ITU-R BT.2100-0
        smpte428    SMPTE ST 428-1
        xyz         (CIE 1931 XYZ as in ISO 11664-1)
        smpte431    SMPTE RP 431-2 (2011)
        smpte432    SMPTE EG 432-1 (2010)
        22          No corresponding industry specification identified
        --transfer-characteristics
                    Set transfer characteristics information value.

        <Value defined in H.273>
                    See https://www.itu.int/rec/T-REC-H.273-201612-I/en

        bt709       Rec. ITU-R BT.709-6
        unspecified Image characteristics are unknown or are determined by the application.
        bt470m      Rec. ITU-R BT.470-6 System M (historical)
        bt470bg     Rec. ITU-R BT.470-6 System B, G (historical)
        bt601       Rec. ITU-R BT.1700-0 NTSC
        ntsc        Rec. ITU-R BT.1700-0 NTSC
        smpte240m   SMPTE 240M (1999) (historical)
        linear      Linear transfer characteristics
        log100      Logarithmic transfer characteristic (100:1 range)
        log100sqrt10
                    Logarithmic transfer characteristic (100 * Sqrt( 10 ) : 1 range)

        iec61966    IEC 61966-2-4
        bt1361      Rec. ITU-R BT.1361-0 extended colour gamut system (historical)
        sRGB        IEC 61966-2-1 sRGB or sYCC
        sYCC        IEC 61966-2-1 sRGB or sYCC
        bt2020      Rec. ITU-R BT.2020-2 (10-bit system)
        bt2020-10bit
                    Rec. ITU-R BT.2020-2 (10-bit system)

        bt2020-12bit
                    Rec. ITU-R BT.2020-2 (12-bit system)

        smpte2084   SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems
        bt2100pq    Rec. ITU-R BT.2100-0 perceptual quantization (PQ) system
        smpte428    SMPTE ST 428-1
        bt2100hlg   Rec. ITU-R BT.2100-0 hybrid log-gamma (HLG) system
        arib-b67    ARIB STD-B67
        --matrix-coefficients
                    Set matrix coefficients information value.

        <Value defined in H.273>
                    See https://www.itu.int/rec/T-REC-H.273-201612-I/en

        bt709       Rec. ITU-R BT.709-6 (default)
        sRGB        IEC 61966-2-1 sRGB or sYCC (default)
        unspecified Image characteristics are unknown or are determined by the application
        us-fcc      United States Federal Communications Commission (2003)
        bt470bg     Rec. ITU-R BT.470-6 System B, G (historical)
        sYCC        IEC 61966-2-1 sRGB or sYCC
        bt601       Rec. ITU-R BT.601-7 625
        ntsc        Rec. ITU-R BT.1700-0 NTSC
        smpte240m   SMPTE 240M
        bt2020      Rec. ITU-R BT.2020-2 (non-constant luminance)
        --horizontal-scale-mode
                    Set horizontal scale mode

        1/1         Do not scale (default)
        1/2         Scale to 1/2
        3/5         Scale to 3/5
        4/5         Scale to 4/5
        1/4         Scale to 1/4
        3/4         Scale to 3/4
        1/8         Scale to 1/8
        --vertical-scale-mode
                    Set vertical scale mode

        1/1         Do not scale (default)
        1/2         Scale to 1/2
        3/5         Scale to 3/5
        4/5         Scale to 4/5
        1/4         Scale to 1/4
        3/4         Scale to 3/4
        1/8         Scale to 1/8
        --resize-mode
                    Set resize mode

        none        Do not resize
        fixed       Resize image using a denominator_ given by `--resize-denominator_` arg
        random      Resize image randomly!
        --resize-denominator_
                    Set resize denominator_.

        --superres-mode
                    Set superres mode

        none        Do not use superres mode
        fixed       Apply superres filter to image using a denominator_ given by
                    `--superres-denominator_` arg

        random      Apply superres filter to image with a random denominator_!
        qthresh     Apply or do not apply superres filter to image based on the q index
        auto        Apply or do not apply superres filter to image automatically
        --superres-denominator_
                    Set superres resize denominator_.

        --superres-qthresh
                    Set q level threshold for superres.

        --render-width
                    Set render width explicitly

        --render-height
                    Set render height explicitly

        --profile   AV1 Profile(0=base, 1=high, 2=professional)
        --pix-fmt   Pixel format of output image
        yuv420      YUV420 format (default)
        yuv422      YUV422 format
        yuv444      YUV444 format (recommended for lossless images)
        --bit-depth Bit depth of output image
        8           8bits per color, 24bits per pixel (default)
        10          10bits per color, 30bits per pixel
        12          12bits per color, 36bits per pixel
        --disable-full-color-range
                    Use limited YUV color range (default)

        --enable-full-color-range
                    Use full YUV color range

        --encoder-usage
                    Encoder usage

        good        Good Quality mode (default)
        realtime    Real time encoding mode
        --enable-row-mt
                    Enable row based multi-threading of encoder

        --disable-row-mt
                    Disable row based multi-threading of encoder (default)

        --cpu-used  Quality/Speed ratio modifier
        --rate-control
                    Rate control method

        cbr         Constant Bit Rate mode. Please also set `--bit-rate` arg.
        q           Constant Quality (default)
        cq          Constrained Quality
        --bit-rate  Bit rate of output image.
        --crf       CQ Level in CQ rate control mode
        --qmin      Minimum (Best Quality) Quantizer
        --qmax      Maximum (Worst Quality) Quantizer
        --adaptive-quantization
                    Set adaptive-quantization mode

        none        none(default)
        variance    variance based
        complexity  complexity based
        cyclic      Cyclic refresh
        --enable-adaptive-quantization-b
                    use adaptive quantize_b

        --disable-adaptive-quantization-b
                    use traditional adaptive quantization (default)

        --delta-q   a mode of delta q mode feature, that allows modulating q per superblock
        none        disable deltaQ
        objective   Use modulation to maximize objective quality
        perceptual  Use modulation to maximize perceptual quality
        --delta-q-strength
                    strength of deltaQ [0..1000] (default = 100)

        --enable-chroma-delta-q
                    enable delta quantization in chroma

        --disable-chroma-delta-q
                    disable delta quantization in chroma

        --enable-loop-filter
                    enable loop filter (default)

        --disable-loop-filter
                    disable loop filter

        --enable-delta-lf
                    enable delta loop filter

        --disable-delta-lf
                    disable delta loop filter (default)

        --use-qm    Use QMatrix
        --qm-min    Min quant matrix flatness
        --qm-max    Max quant matrix flatness
        --qm-min-y  Min quant matrix flatness for Y
        --qm-min-u  Min quant matrix flatness for U
        --qm-min-v  Min quant matrix flatness for V
        --tune      Quality metric to tune
        ssim        SSIM(structural similarity)
        psnr        PSNR(peak signal-to-noise ratio)
        vmaf-with-preprocessing
                    vmaf-with-preprocessing

        vmaf-without-preprocessing
                    vmaf-without-preprocessing

        vmaf-max-gain
                    vmaf-max-gain

        vmaf-neg-max-gain
                    vmaf-neg-max-gain

        --content-type
                    Content type

        default     Regular video content (default)
        screen      Screen capture content
        film        Film content
        --vmaf-model-path
                    VMAF model file path to tuning image quality

        --lossless  Enable lossless encoding
        --monochrome
                    Encode to monochrome image

        --sharpness Sharpening output
        --disable-cdef
                    Disable Constrained Directional Enhancement Filter (default)

        --enable-cdef
                    Enable Constrained Directional Enhancement Filter

        --disable-loop-restoration
                    Disable Loop Restoration Filter (default)

        --enable-loop-restoration
                    Enable Loop Restoration Filter

        --superblock-size
                    Superblock size.

        dynamic     encoder determines the size automatically.
        128         use 128x128 superblock.
        64          use 64x64 superblock.
        --tile-rows Number of tile rows
        --tile-columns
                    Number of tile columns

        --keyframe-temporal-filter
                    Enable temporal filtering on key frame

        --enable-rect-partitions
                    enable rectangular partitions (default)

        --disable-rect-partitions
                    disable rectangular partitions

        --enable-ab-partitions
                    enable ab partitions (default)

        --disable-ab-partitions
                    disable ab partitions

        --disable-1to4-partitions
                    enable 1to4 partitions (default)

        --enable-1to4-partitions
                    disable 1to4 partitions

        --enable-intra-edge-filter
                    enable intra edge filter (default)

        --disable-intra-edge-filter
                    disable intra edge filter

        --min-partition-size
                    min partition size

        --max-partition-size
                    max partition size

        --enable-tx64
                    enable 64-length transforms (default)

        --disable-tx64
                    disable 64-length transforms

        --enable-flip-idtx
                    enable flip and identity transforms (default)

        --disable-flip-idtx
                    disable flip and identity transforms

        --enable-rect-tx
                    enable rectangular transforms (default)

        --disable-rect-tx
                    disable rectangular transforms

        --use-dct-only
                    Use DCT tx onlyq

        --use-default-tx-only
                    use default tx type only

        --use-reduced-tx-set
                    use reduced tx set, transforms w/o flip (4) + Identity (1).

        --enable-filter-intra
                    enable (default)

        --disable-filter-intra
                    disable 

        --enable-smooth-intra
                    enable (default)

        --disable-smooth-intra
                    disable 

        --enable-paeth-intra
                    enable (default)

        --disable-paeth-intra
                    disable 

        --enable-chroma-from-luma
                    enable (default)

        --disable-chroma-from-luma
                    disable 

        --enable-superres
                    enable frame superresolution (default)

        --disable-superres
                    disable frame superresolution

        --enable-palette
                    enable palette mode

        --disable-palette
                    disable palette mode (default)

        --enable-intrabc
                    enable intra block copy mode (default)

        --disable-intrabc
                    disable intra block copy mode

        --enable-angle-delta
                    enable intra angle delta (default)

        --disable-angle-delta
                    disable intra angle delta

        --enable-diagonal-intra
                    enable usage of D45 to D203 intra modes (default)

        --disable-diagonal-intra
                    disable usage of D45 to D203 intra modes

        --enable-directional-intra
                    turn on directional intra mode (default)

        --disable-directional-intra
                    turn off directional intra mode

        --enable-tx-size-search
                    turn on transform size search (default). Transforms always have the largest
                    possible size

        --disable-tx-size-search
                    turn off transform size search. Search for the best transform size for each
                    block

        -h, --help  Show help and exit.
```

## TODO

 - Thumbnail
 - Improve default configs
 - Add more and more command-line flags.

# Related repositories

 - [link-u/davif](https://github.com/link-u/davif) - avif decoder, using dav1d directly.
 - [link-u/libavif-container](https://github.com/link-u/libavif-container) - a library to parse avif container.
 - [link-u/avif-sample-images](https://github.com/link-u/avif-sample-images) - sample images from us.
