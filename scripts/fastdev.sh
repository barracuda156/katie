#!/bin/sh

set -e

cwd="$(dirname $0)"

rm -rf "$cwd/../fastdev"
mkdir -p "$cwd/../fastdev"
cd "$cwd/../fastdev"

cmake ../ -DCMAKE_BUILD_TYPE=FastDev -DKATIE_ALLINONE=ON -DCMAKE_INSTALL_PREFIX=/usr $@
make -j$(nproc)
