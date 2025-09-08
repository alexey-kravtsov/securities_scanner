#!/bin/bash

BUILD_PARAMS="--reconfigure -Ddefault_library=static -Dcpp_std=c++23"

if [ "$1" == "release" ]; then
  BUILD_PARAMS+=" -Dbuildtype=release"
fi

meson setup builddir $BUILD_PARAMS
meson compile -C builddir