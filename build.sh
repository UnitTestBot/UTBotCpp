#!/bin/bash
#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

set -e
set -o pipefail
pwd=$PWD
chmod +x $pwd/klee/build.sh $pwd/Bear/build.sh $pwd/server/build.sh
cd $pwd/klee && ./build.sh
cd $pwd/Bear && ./build.sh
cd $pwd/server && ./build.sh
