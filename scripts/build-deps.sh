#! /bin/bash -eux

function readlink_f() {
  local src='import os,sys;print(os.path.realpath(sys.argv[1]))'
  python3 -c "${src}" "$1" || python -c "${src}" "$1"
}

ROOT_DIR="$(cd "$(readlink_f "$(dirname "$0")")" && cd .. && pwd)"
cd "${ROOT_DIR}" || exit 1

set -eux
set -o pipefail

DEPS_DIR="$(readlink_f "${ROOT_DIR}/_deps")"

rm -Rf "${DEPS_DIR}"
mkdir -p "${DEPS_DIR}"

# zlib
case "$(uname -s)" in
    MINGW*)
bash -eux <<EOF
cd external/zlib
BINARY_PATH="${DEPS_DIR}/bin" INCLUDE_PATH="${DEPS_DIR}/include" LIBRARY_PATH="${DEPS_DIR}/lib" \
make -f win32/Makefile.gcc
EOF
      ;;
    *)
bash -eux <<EOF
cd external/zlib
./configure "--prefix=${DEPS_DIR}" --static
make
make install
make clean
EOF
esac

# libpng
bash -eux <<EOF
cd external/libpng
CPPFLAGS="-I${DEPS_DIR}/include" LDFLAGS="-L${DEPS_DIR}/lib" \
./configure "--prefix=${DEPS_DIR}" --enable-static --disable-shared
make
make install
EOF

# libvmaf
bash -eux <<EOF
cd external/vmaf/libvmaf
rm -Rf build
meson setup \
  "--prefix=${DEPS_DIR}" \
  "--libdir" "lib" \
  "--auto-features" "disabled" \
  "--backend" "ninja" \
  "--buildtype" "release" \
  "--default-library" "static" \
  "--optimization" "3" \
  "-Denable_asm=true" \
  "-Denable_tests=false" \
  "build" \
  "."
(cd build && ninja && meson install)
EOF
