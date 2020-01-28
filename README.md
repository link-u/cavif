# cavif

avif encoder, using [libaom](https://aomedia.googlesource.com/aom/) directly.

The most significant mission of this project is "**Make it enable to tune libaom's all encoding options to optimize quality/size ratio as nice as possible human beings can**".

## basic usage

```bash
cavif -i <output.png> -o <input.avif>
```

Example avif files are available in [AOMediaCodec/av1-avif](https://github.com/AOMediaCodec/av1-avif/tree/master/testFiles) or [link-u/avif-sample-images](https://github.com/link-u/avif-sample-images).

 - [日本語のドキュメント](./doc/ja_JP/README.md)

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

## SYNOPSIS

```
% cavif
[2020/01/28 14:38:40 INFO ] cavif
[2020/01/28 14:38:40 INFO ] libaom ver: 1.0.0-errata1-avif
SYNOPSIS
        cavif -i <input.png> -o <output.avif> [--rotation [0|90|180|270]] [--mirror
              [vertical|horizontal]] [--crop-size <widthN/widthD,heightN/heightD>] [--crop-offset
              <horizOffN/horizOffD,vertOffN/vertOffD>] [--full-still-picture-header] [--profile
              <0=base, 1=high, 2=professional>] [--pix-fmt [yuv420|yuv422|yuv444]] [--bit-depth
              [8|10|12]] [--disable-full-color-range] [--enable-full-color-range] [--encoder-usage
              [good|realtime]] [--threads <Num of threads to use>] [--row-mt] [--cpu-used <0-8>]
              [--rate-control [q|cq]] [--bit-rate <kilo-bits per second>] [--crf <0-63>] [--qmin
              <0-63>] [--qmax <0-63>] [--adaptive-quantization [none|variance|complexity|cyclic]]
              [--enable-adaptive-quantization-b] [--disable-adaptive-quantization-b] [--delta-q
              [none|objective|perceptual]] [--enable-chroma-delta-q] [--disable-chroma-delta-q]
              [--enable-delta-lf] [--disable-delta-lf] [--use-qm] [--qm-min <0-15 (default: 5)>]
              [--qm-max <0-15 (default: 9)>] [--qm-min-y <0-15 (default: 10)>] [--qm-min-u <0-15
              (default: 11)>] [--qm-min-v <0-15 (default: 12)>] [--tune
              [ssim|psnr|cdef-dist|daala-dist]] [--lossless] [--monochrome] [--sharpness <0-7>]
              [--disable-cdef] [--enable-cdef] [--disable-loop-restoration]
              [--enable-loop-restoration] [--superblock-size [dynamic|128|64]] [--tile-rows <0-6>]
              [--tile-colums <0-6>] [--disable-keyframe-temporal-filtering]
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

        --rotation  Set rotation meta data(irot). Counter-clockwise.
        --mirror    Set mirror meta data(imir).
        --crop-size Set crop size.
        --crop-offset
                    Set crop offset.

        --full-still-picture-header
                    Force to output full picture header

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
        cdef-dist   cdef-dist
        daala-dist  daala-dist
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
        --tile-colums
                    Number of tile colums

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

 - Support:
   - Alpha channel(Auxially images)
   - Color profiles
 - Add more and more command-line flags.

# Related repositories

 - [link-u/davif](https://github.com/link-u/davif) - avif decoder, using dav1d directly.
 - [link-u/libavif-container](https://github.com/link-u/libavif-container) - a library to parse avif container.
 - [link-u/avif-sample-images](https://github.com/link-u/avif-sample-images) - sample images from us.
