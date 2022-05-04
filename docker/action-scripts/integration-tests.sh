#!/bin/bash

#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

source docker/building_dependencies/runtime_env.sh
cd vscode-plugin

#download vs_code
npm run download_vscode

chmod +x src/test/run.sh
./src/test/run.sh
