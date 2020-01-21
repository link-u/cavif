# cavif

avif encoder, using [libaom](https://aomedia.googlesource.com/aom/) directly.

## basic usage

```bash
cavif -i <output.png> -o <input.avif>
```

Example avif files are available in [AOMediaCodec/av1-avif](https://github.com/AOMediaCodec/av1-avif/tree/master/testFiles) or [link-u/avif-sample-images](https://github.com/link-u/avif-sample-images).

## how to build

```bash
# cloning this repository with dependencies.
git clone --recurse-submodules --recursive git@link-u.github.com:link-u/cavif.git
cd cavif

# build davif
mkdir build && cd build
cmake ..
make
```

## SYNOPSIS

```
% cmake-build-debug/cavif
SYNOPSIS
        cavif -i <input.png> -o <output.avif> [--profile <0=base, 1=high, 2=professional>]
              [--monochrome] [--usage [good|realtime]] [--threads <Num of threads to use>]
              [--pix-fmt [yuv420|yuv422|yuv444]] [--bit-depth [8|10|12]] [--rate-control [q|cq]]
              [--enable-large-scale-tile] [--disable-large-scale-tile] [--full-still-picture-header]
              [--enable-full-color-range] [--disable-full-color-range] [--crf <0-63>] [--cpu-used
              <0-8>] [--enable-cdef] [--disable-cdef] [--superblock-size [dynamic|128|64]] [--tune
              [ssim|psnr|cdef-dist|daala-dist]]

OPTIONS
        -i, --input Filename to input
        -o, --output
                    Filename to output

        --profile   AV1 Profile(0=base, 1=high, 2=professional)
        --monochrome
                    Encode to monochrome image.

        --usage     Encoder usage
        good        Good Quality mode
        realtime    Real time encoding mode.
        --pix-fmt   Pixel format of output image.
        --bit-depth Bit depth of output image.
        --rate-control
                    Rate control method

        q           Constant Quality
        cq          Constrained Quality
        --enable-large-scale-tile
                    Use large scale tile mode.

        --disable-large-scale-tile
                    Don't use large scale tile mode.

        --full-still-picture-header
                    Force to output full picture header

        --enable-full-color-range
                    Use full YUV color range.

        --disable-full-color-range
                    Use limited YUV color range.

        --crf       CQ Level in CQ rate control mode
        --cpu-used  Quality/Speed ratio modifier
        --enable-cdef
                    Enable Constrained Directional Enhancement Filter

        --disable-cdef
                    Disable Constrained Directional Enhancement Filter

        --superblock-size
                    Superblock size.

        dynamic     encoder determines the size automatically.
        128         use 128x128 superblock.
        64          use 64x64 superblock.
        --tune      Quality metric to tune
        ssim        structural similarity
        psnr        peak signal-to-noise ratio
        cdef-dist   cdef-dist
        daala-dist  daala-dist
```

## TODO

 - Support:
   - Alpha channel(Auxially images)
   - Color profiles
   - HDR images(10bit/12bit).
 - Add more and more command-line flags.

# Related repositories

 - [link-u/davif](https://github.com/link-u/davif) - avif decoder, using dav1d directly.
 - [link-u/libavif-container](https://github.com/link-u/libavif-container) - a library to parse avif container.
 - [link-u/avif-sample-images](https://github.com/link-u/avif-sample-images) - sample images from us.
