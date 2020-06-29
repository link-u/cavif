#! /bin/bash -eux

set -eux
set -o pipefail

BASE_DIR=$(cd $(dirname $(readlink -f $0)) && cd .. && pwd)
cd ${BASE_DIR}

apt install -y ./artifact/*.deb
apt show cavif
which cavif

cavif || true
if [ $? -ne 0 ]; then
  exit -1
fi

ldd $(which cavif)
