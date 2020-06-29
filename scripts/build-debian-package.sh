#! /bin/bash -eux

set -eux

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
apt-get install -y --no-install-recommends apt-transport-https ca-certificates gnupg software-properties-common wget
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"

# Workaround: gcc >= 8.0 is required.
case $(lsb_release -cs) in
  bionic)
      export CC=gcc-8
      export CXX=g++-8
      sed -i -r "s/gcc-9/gcc-8/g"                       "${BASE_DIR}/debian/control"
      sed -i -r "s/g\+\+-9/g++-8/g"                     "${BASE_DIR}/debian/control"
      sed -i -r "s/libstdc\+\+-9-dev/libstdc++-8-dev/g" "${BASE_DIR}/debian/control"
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
mk-build-deps --install --remove \
  --tool='apt-get -o Debug::pkgProblemResolver=yes --no-install-recommends --yes' \
  "${BASE_DIR}/debian/control"

fakeroot debian/rules clean
fakeroot debian/rules build
fakeroot debian/rules binary
# workaround. external/libpng will be dirty after making debian packages.
env --chdir=external/libpng git reset --hard
