#!/bin/bash

#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

mkdir -p build
cd build
$UTBOT_CMAKE_BINARY -G "Ninja" -DCMAKE_INSTALL_PREFIX=$UTBOT_ALL/server-install ..
$UTBOT_CMAKE_BINARY --build .
$UTBOT_CMAKE_BINARY --install .