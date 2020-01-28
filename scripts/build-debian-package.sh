#! /bin/bash
set -e

BASE_DIR=$(cd $(dirname $0); cd ..; pwd)
cd ${BASE_DIR}

fakeroot debian/rules clean
fakeroot debian/rules binary
# workaround. external/libpng will be dirty after making debian packages.
env --chdir=external/libpng git reset --hard
mv ../cavif_*.deb ../cavif-dbgsym_*.ddeb .
