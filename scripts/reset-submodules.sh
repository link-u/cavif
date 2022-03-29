#! /bin/bash -eux

function readlink_f() {
  local src='import os,sys;print(os.path.realpath(sys.argv[1]))'
  python3 -c "${src}" "$1" || python -c "${src}" "$1"
}

ROOT_DIR="$(cd "$(readlink_f "$(dirname "$0")")" && cd .. && pwd)"
cd "${ROOT_DIR}" || exit 1

set -eux
set -o pipefail

git submodule foreach 'git reset --hard && git clean -fdx'
