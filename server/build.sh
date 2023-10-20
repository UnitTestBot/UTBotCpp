#!/bin/bash

mkdir -p build
cd build

$UTBOT_CMAKE_BINARY -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$UTBOT_ALL/server-install ..
$UTBOT_CMAKE_BINARY --build .
$UTBOT_CMAKE_BINARY --install .
