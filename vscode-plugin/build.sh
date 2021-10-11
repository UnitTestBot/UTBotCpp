#!/bin/bash
#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

set -e
set -o pipefail
utbot_dir="$(dirname $PWD)"
cp -r $UTBOT_ALL/node_modules .
npm install
cd $utbot_dir/vscode-plugin \
       	&& npm rebuild grpc --runtime=electron --target=7.3.0 \
	&& ./protoc.sh $utbot_dir/server/proto $utbot_dir/vscode-plugin/src/proto-ts
