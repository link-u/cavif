#! /bin/bash
set -e

BASE_DIR=$(cd $(dirname $(readlink -f $0)) && cd .. && pwd)
cd ${BASE_DIR}

# Generate changelog
git_describe="$(git describe --tags)"
VERSION=${git_describe:1}.$(TZ=JST-9 date +%Y%m%d)+$(lsb_release -cs)
DATE=$(LC_ALL=C TZ=JST-9 date '+%a, %d %b %Y %H:%M:%S %z')

cat <<EOF > "${BASE_DIR}/debian/changelog"
cavif (${VERSION}) unstable; urgency=medium

  * This is atomated build.
  * Please see https://github.com/link-u/cavif/releases for more information!

 -- Ryo Hirafuji <ryo.hirafuji@link-u.co.jp>  ${DATE}
EOF

# Add Kitware APT repository to install the latest cmake.
# https://apt.kitware.com/
apt-get update
apt-get install apt-transport-https ca-certificates gnupg software-properties-common wget
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"

# Install deps to build.
mk-build-deps --install --remove \
  --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' \
  "${BASE_DIR}/debian/control"

fakeroot debian/rules clean
fakeroot debian/rules build
fakeroot debian/rules binary
# workaround. external/libpng will be dirty after making debian packages.
env --chdir=external/libpng git reset --hard
mv ../cavif_*.deb ../cavif-dbgsym_*.ddeb .
