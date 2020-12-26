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

cavifは、ラッパーを介さず[libaom](https://aomedia.googlesource.com/aom/)を直接叩くavifのエンコーディング・コマンドです。このプロジェクトの唯一にして最大の使命は、 **libaomの静止画に関する全エンコードオプションを仔細に操作できるようにして、現生人類が現時点で達成可能な最高の圧縮効率を実現する手段を提供すること** です。

[日本語の詳しいドキュメントはこちら](./doc/ja_JP/README.md)。

# Usage

## how to build

```bash
# pre-requirements
# If your system cmake is lower than 3.13, please install latest version:
# https://apt.kitware.com/

# cloning this repository with dependencies.
git clone --recurse-submodules --recursive git@github.com:link-u/cavif.git

cd cavif

# make build directory
mkdir build && cd build

# System gcc is 8.0 or higher:
cmake ..

# If not, please install gcc-8 (or higher) and tell them to CMake.
CXX=g++-8 CC=gcc-8 cmake ..

# build cavif binary.
make cavif
```

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
% cavif
[2020/03/18 16:04:09 INFO ] cavif
[2020/03/18 16:04:09 INFO ] libaom ver: 1.0.0-errata1-avif-414-g0cfdca944
SYNOPSIS
        cavif -i <input.png> -o <output.avif> [--attach-alpha <input-alpha.avif>] [--attach-depth
              <input-depth.avif>] [--encode-target [image|alpha]] [--show-result] [--rotation
              [0|90|180|270]] [--mirror [vertical|horizontal]] [--crop-size
              <widthN/widthD,heightN/heightD>] [--crop-offset
              <horizOffN/horizOffD,vertOffN/vertOffD>] [--full-still-picture-header]
              [--color-primaries (<Value defined in
              H.273>|bt709|sRGB|sYCC|unspecified|bt470m|bt470bg|bt601|ntsc|smpte240m|generic-film|bt2020|bt2100|xyz|smpte428|smpte431|smpte432|ebu3213)]
              [--transfer-characteristics (<Value defined in
              H.273>|bt709|unspecified|bt470m|bt470bg|bt601|ntsc|smpte240m|linear|log100|log100sqrt10|iec61966|bt1361|sRGB|sYCC|bt2020|bt2020-10bit|bt2020-12bit|smpte2084|bt2100pq|smpte428|bt2100hlg|arib-b67)]
              [--matrix-coefficients (<Value defined in
              H.273>|bt709|sRGB|sYCC|unspecified|us-fcc|bt470bg|bt601|ntsc|smpte240m|bt2020)]
              [--horizontal-scale-mode [1/1|4/5|3/5|1/2]] [--vertical-scale-mode [1/1|4/5|3/5|1/2]]
              [--resize-mode [none|fixed|random]] [--resize-denominator <[8-16]>] [--superres-mode
              [none|fixed|random|qthresh|auto]] [--superres-denominator <[8-16]>]
              [--superres-qthresh <[0-63]>] [--render-width <<render-width>>] [--render-height
              <<render-height>>] [--profile <0=base, 1=high, 2=professional>] [--pix-fmt
              [yuv420|yuv422|yuv444]] [--bit-depth [8|10|12]] [--disable-full-color-range]
              [--enable-full-color-range] [--encoder-usage [good|realtime]] [--threads <Num of
              threads to use>] [--row-mt] [--cpu-used <0-8>] [--rate-control [q|cq]] [--bit-rate
              <kilo-bits per second>] [--crf <0-63>] [--qmin <0-63>] [--qmax <0-63>]
              [--adaptive-quantization [none|variance|complexity|cyclic]]
              [--enable-adaptive-quantization-b] [--disable-adaptive-quantization-b] [--delta-q
              [none|objective|perceptual]] [--enable-chroma-delta-q] [--disable-chroma-delta-q]
              [--enable-delta-lf] [--disable-delta-lf] [--use-qm] [--qm-min <0-15 (default: 5)>]
              [--qm-max <0-15 (default: 9)>] [--qm-min-y <0-15 (default: 10)>] [--qm-min-u <0-15
              (default: 11)>] [--qm-min-v <0-15 (default: 12)>] [--tune
              [ssim|psnr|vmaf-with-preprocessing|vmaf-without-preprocessing|vmaf-max-gain]]
              [--vmaf-model-path <<path-to-vmaf-model-file>>] [--lossless] [--monochrome]
              [--sharpness <0-7>] [--disable-cdef] [--enable-cdef] [--disable-loop-restoration]
              [--enable-loop-restoration] [--superblock-size [dynamic|128|64]] [--tile-rows <0-6>]
              [--tile-columns <0-6>] [--disable-keyframe-temporal-filtering]
              [--enable-keyframe-temporal-filtering] [--enable-rect-partitions]
              [--disable-rect-partitions] [--enable-ab-partitions] [--disable-ab-partitions]
              [--disable-1to4-partitions] [--enable-1to4-partitions] [--enable-intra-edge-filter]
              [--disable-intra-edge-filter] [--min-partition-size [4|8|16|32|64|128]]
              [--max-partition-size [4|8|16|32|64|128]] [--enable-tx64] [--disable-tx64]
              [--enable-flip-idtx] [--use-dct-only] [--use-default-tx-only] [--use-reduced-tx-set]
              [--enable-filter-intra] [--disable-filter-intra] [--enable-smooth-intra]
              [--disable-smooth-intra] [--enable-paeth-intra] [--disable-paeth-intra]
              [--enable-chroma-from-luma] [--disable-chroma-from-luma] [--enable-superres]
              [--disable-superres] [--enable-palette] [--disable-palette] [--enable-intrabc]
              [--disable-intrabc] [--enable-angle-delta] [--disable-angle-delta]

OPTIONS
        -i, --input Filename to input
        -o, --output
                    Filename to output

        --attach-alpha
                    Attach alpha plane.

        --attach-depth
                    Attach depth plane.

        --encode-target
                    Encode alpha image.

        image       Encode image planes
        alpha       Encode alpha planes
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
        smpte240m   SMPTE 240M (1999) (historical)
        generic-film
                    Generic film (colour filters using Illuminant C)

        bt2020      Rec. ITU-R BT.2020-2
        bt2100      Rec. ITU-R BT.2100-0
        xyz         (CIE 1931 XYZ as in ISO 11664-1)
        smpte428    SMPTE ST 428-1
        smpte431    SMPTE RP 431-2 (2011)
        smpte432    SMPTE EG 432-1 (2010)
        ebu3213     EBU Tech. 3213-E (1975)
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

        bt709       Rec. ITU-R BT.709-6
        sRGB        IEC 61966-2-1 sRGB or sYCC
        sYCC        IEC 61966-2-1 sRGB or sYCC
        unspecified Image characteristics are unknown or are determined by the application
        us-fcc      United States Federal Communications Commission (2003)
        bt470bg     Rec. ITU-R BT.470-6 System B, G (historical)
        bt601       Rec. ITU-R BT.601-7 625
        ntsc        Rec. ITU-R BT.1700-0 NTSC
        smpte240m   SMPTE 240M
        bt2020      Rec. ITU-R BT.2020-2 (non-constant luminance)
        --horizontal-scale-mode
                    Set horizontal scale mode

        --vertical-scale-mode
                    Set vertical scale mode

        --resize-mode
                    Set resize mode

        --resize-denominator
                    Set resize denominator.

        --superres-mode
                    Set resize mode

        --superres-denominator
                    Set resize denominator.

        --superres-qthresh
                    Set q level threshold for superres.

        --render-width
                    Set render width.

        --render-height
                    Set render height.

        --profile   AV1 Profile(0=base, 1=high, 2=professional)
        --pix-fmt   Pixel format of output image.
        --bit-depth Bit depth of output image.
        --disable-full-color-range
                    Use limited YUV color range.

        --enable-full-color-range
                    Use full YUV color range.

        --encoder-usage
                    Encoder usage

        good        Good Quality mode
        realtime    Real time encoding mode.
        --row-mt    Enable row based multi-threading of encoder
        --cpu-used  Quality/Speed ratio modifier
        --rate-control
                    Rate control method

        q           Constant Quality
        cq          Constrained Quality
        --bit-rate  Bit rate of output image.
        --crf       CQ Level in CQ rate control mode
        --qmin      Minimum (Best Quality) Quantizer
        --qmax      Maximum (Worst Quality) Quantizer
        --adaptive-quantization
                    Set adaptive-quantization mode

        none        none
        variance    variance based
        complexity  complexity based
        cyclic      Cyclic refresh
        --enable-adaptive-quantization-b
                    use adaptive quantize_b

        --disable-adaptive-quantization-b
                    use traditional adaptive quantization

        --delta-q   a mode of delta q mode feature, that allows modulating q per superblock
        none        disable deltaQ
        objective   Use modulation to maximize objective quality
        perceptual  Use modulation to maximize perceptual quality
        --enable-chroma-delta-q
                    enable delta quantization in chroma

        --disable-chroma-delta-q
                    disable delta quantization in chroma

        --enable-delta-lf
                    enable delta loop filter

        --disable-delta-lf
                    disable delta loop filter

        --use-qm    Use QMatrix
        --qm-min    Min quant matrix flatness
        --qm-max    Max quant matrix flatness
        --qm-min-y  Min quant matrix flatness for Y
        --qm-min-u  Min quant matrix flatness for U
        --qm-min-v  Min quant matrix flatness for V
        --tune      Quality metric to tune
        ssim        structural similarity
        psnr        peak signal-to-noise ratio
        vmaf-with-preprocessing
                    vmaf-with-preprocessing

        vmaf-without-preprocessing
                    vmaf-without-preprocessing

        vmaf-max-gain
                    vmaf-max-gain

        --vmaf-model-path
                    VMAF model file path to tuning image quality.

        --lossless  Enable lossless encoding
        --monochrome
                    Encode to monochrome image.

        --sharpness Sharpening output
        --disable-cdef
                    Disable Constrained Directional Enhancement Filter

        --enable-cdef
                    Enable Constrained Directional Enhancement Filter

        --disable-loop-restoration
                    Disable Loop Restoration Filter

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

        --disable-keyframe-temporal-filtering
                    Disable temporal filtering on key frame

        --enable-keyframe-temporal-filtering
                    Enable temporal filtering on key frame

        --enable-rect-partitions
                    enable rectangular partitions

        --disable-rect-partitions
                    disable rectangular partitions

        --enable-ab-partitions
                    enable ab partitions

        --disable-ab-partitions
                    disable ab partitions

        --disable-1to4-partitions
                    enable 1to4 partitions

        --enable-1to4-partitions
                    disable 1to4 partitions

        --enable-intra-edge-filter
                    enable intra edge filter

        --disable-intra-edge-filter
                    disable intra edge filter

        --min-partition-size
                    min partition size

        --max-partition-size
                    max partition size

        --enable-tx64
                    enable 64-length transforms

        --disable-tx64
                    disable 64-length transforms

        --enable-flip-idtx
                    enable flip and identity transforms.

        --use-dct-only
                    use dct only.

        --use-default-tx-only
                    use default tx type only

        --use-reduced-tx-set
                    use reduced tx set, transforms w/o flip (4) + Identity (1).

        --enable-filter-intra
                    enable 

        --disable-filter-intra
                    disable 

        --enable-smooth-intra
                    enable 

        --disable-smooth-intra
                    disable 

        --enable-paeth-intra
                    enable 

        --disable-paeth-intra
                    disable 

        --enable-chroma-from-luma
                    enable 

        --disable-chroma-from-luma
                    disable 

        --enable-superres
                    enable frame superresolution

        --disable-superres
                    disable frame superresolution

        --enable-palette
                    enable palette mode

        --disable-palette
                    disable palette mode

        --enable-intrabc
                    enable intra block copy mode

        --disable-intrabc
                    disable intra block copy mode

        --enable-angle-delta
                    enable intra angle delta

        --disable-angle-delta
                    disable intra angle delta
```

## TODO

 - Thumbnail
 - Improve default configs
 - Add more and more command-line flags.

# Related repositories

 - [link-u/davif](https://github.com/link-u/davif) - avif decoder, using dav1d directly.
 - [link-u/libavif-container](https://github.com/link-u/libavif-container) - a library to parse avif container.
 - [link-u/avif-sample-images](https://github.com/link-u/avif-sample-images) - sample images from us.
