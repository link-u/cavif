#! /bin/bash -eux

set -eux

cd $(cd $(dirname $(readlink -f $0)) && .. && pwd)

apt install -y ./artifact/*.deb
apt show cavif
which cavif

cavif --help

ldd $(which cavif)
