# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.4.0]

### Added

- New parameters for `--horizontal-scale-mode` and `--vertical-scale-mode` are introduced.
  - `1/4`
  - `3/4`
  - `1/8`
- `--cpu-used 9` is introduced.
- `--enable-rect-tx` is introduced. Enabled by default.
- `--(enable|disable)-diagonal-intra` is introduced. Enabled by default.
- `--content-type (default|screen|film)` is introduced.
  - `default`: regular video content(default)
  - `screen`: Screen capture content
  - `film`: Film content
- `--(enable|disable)-tx-size-search` is introduced. Enabled by default.
- `--delta-q-strength (int)` is introduced. Takes 0 to 1000. Default is `100`.
- `--(enable|disble)-loop-filter` is introduced. Enabled by default.

### Changed
- libaom is upgraded to v3.3.0
- libvmaf is upgrade to [441ab02a6b9df77a716f9bc1772340acd36e201b](https://github.com/Netflix/vmaf/tree/441ab02a6b9df77a716f9bc1772340acd36e201b).
- [Remove URL from header to make image sizes smaller](https://github.com/link-u/cavif/pull/56).
- Default values of `--color-primaries`, `--transfer-characteristics` and `--matrix-coefficients` are now "empty".
  - If those flags are not set and input PNG file does not contain a `sRGB` chunk, these settings are used:
    - `--color-primaries 1` (sRGB/sYCC)
    - `--transfer-characteristics 13` (sRGB/sYCC)
    - `--matrix-coefficients 5` (sYCC)
- All of `--color-primaries`, `--transfer-characteristics` and `--matrix-coefficients` nor none of them must be set.
- [Use SingleItemTypeReferenceBox instead of SingleItemTypeReferenceBoxLarge](https://github.com/link-u/cavif/commit/e271be5eddf1259d7a34315ece967a5e95766f49) to make images smaller.

### Fixed

- Now we can build cavif for mac OS and debian package.
- `--crop-size` and `--crop-offset` are now validated according to [ISO/IEC 23000-22:2019/Amd 2:2021](https://www.iso.org/standard/81634.html):
  - crop size must be integers.
  - the leftmost pixel must be positioned in even line if pixel format is `yuv420` or `yuv422`.
  - the topmost pixel must be positioned in even line if pixel format is `yuv420`.

### Removed
