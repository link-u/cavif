#! /bin/bash -eux

function readlink_f() {
  local src='import os,sys;print(os.path.realpath(sys.argv[1]))'
  python3 -c "${src}" "$1" || python -c "${src}" "$1"
}

ROOT_DIR="$(cd "$(readlink_f "$(dirname "$0")")" && cd .. && pwd)"
cd "${ROOT_DIR}" || exit 1

set -eux
set -o pipefail

# libaom
(cd external/libaom; patch -p1 --forward < ../../patches/libaom.patch || true)
(cd external/zlib; patch -p1 --forward < ../../patches/zlib.patch || true)
