#!/bin/bash

mkdir -p build
cd build

if [ -z $PROJECT_VERSION ]; then
  PROJECT_VERSION="0.0.0"
fi

if [ -z $RUN_INFO ]; then
  RUN_INFO="local"
fi

$UTBOT_CMAKE_BINARY -G "Ninja" -DCMAKE_INSTALL_PREFIX=$UTBOT_ALL/server-install -DPROJECT_VERSION=$PROJECT_VERSION -DRUN_INFO=$RUN_INFO ..
$UTBOT_CMAKE_BINARY --build .
$UTBOT_CMAKE_BINARY --install .
