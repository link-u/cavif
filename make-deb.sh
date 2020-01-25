#! /bin/bash
fakeroot debian/rules clean
fakeroot debian/rules binary
env --chdir=external/libpng git reset --hard
mv ../cavif_*.deb ../cavif-dbgsym_*.ddeb .
