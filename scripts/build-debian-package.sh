#! /bin/bash -eux

function readlink_f() {
  local src='import os,sys;print(os.path.realpath(sys.argv[1]))'
  python3 -c "${src}" "$1" || python -c "${src}" "$1"
}

ROOT_DIR="$(cd "$(readlink_f "$(dirname "$0")")" && cd .. && pwd)"
cd "${ROOT_DIR}" || exit 1

set -eux
set -o pipefail

chown "${GID}:${UID}" . -R

# Generate changelog
git_describe="$(git describe --tags)"
VERSION=${git_describe:1}.$(TZ=JST-9 date +%Y%m%d)+$(lsb_release -cs)
DATE=$(LC_ALL=C TZ=JST-9 date '+%a, %d %b %Y %H:%M:%S %z')

cat <<EOF > "${ROOT_DIR}/debian/changelog"
cavif (${VERSION}) unstable; urgency=medium

  * This is automated build.
  * Please see https://github.com/link-u/cavif/releases for more information!

 -- Ryo Hirafuji <ryo.hirafuji@link-u.co.jp>  ${DATE}
EOF

# Add Kitware APT repository to install the latest cmake.
# https://apt.kitware.com/
apt-get update
apt-get install -y --no-install-recommends wget ca-certificates
wget -O - 'https://apt.kitware.com/kitware-archive.sh' | sh
apt-get update
# It fails if kitware's apt repository is broken.
apt-get install -y --no-install-recommends cmake

# Workaround: gcc >= 8.0 is required.
case $(lsb_release -cs) in
  bionic)
      export CC=gcc-8
      export CXX=g++-8
      sed -i -r "s/gcc-9/gcc-8/g"                       "${ROOT_DIR}/debian/control"
      sed -i -r "s/g\+\+-9/g++-8/g"                     "${ROOT_DIR}/debian/control"
      sed -i -r "s/libstdc\+\+-9-dev/libstdc++-8-dev/g" "${ROOT_DIR}/debian/control"
    ;;
  *) ;;
esac

# Workaround: meson has been upgraded so fast, we use the latest versions.
apt-get install -y --no-install-recommends python3-venv python3-pip python3-setuptools
python3 -m venv venv
source venv/bin/activate
pip3 install wheel
pip3 install meson ninja

# Install deps to build.
#dpkg-checkbuilddeps "${ROOT_DIR}/debian/control"
mk-build-deps --install --remove \
  --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' \
  "${ROOT_DIR}/debian/control"

bash scripts/reset-submodules.sh
bash scripts/apply-patches.sh
bash scripts/build-deps.sh

fakeroot debian/rules clean
fakeroot debian/rules build
fakeroot debian/rules binary
# workaround. external/libpng will be dirty after making debian packages.
env --chdir=external/libpng git reset --hard
