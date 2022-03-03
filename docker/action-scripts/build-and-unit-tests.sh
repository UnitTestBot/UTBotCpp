#!/bin/bash

#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

source docker/building_dependencies/runtime_env.sh
chmod +x docker/action-scripts/build-utbot.sh
./docker/action-scripts/build-utbot.sh
cd server/build
chmod +x UTBot_UnitTests
./UTBot_UnitTests --verbosity info --log `pwd`
