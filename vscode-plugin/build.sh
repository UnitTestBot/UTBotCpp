#!/bin/bash
set -e
set -o pipefail
utbot_dir="$(dirname $PWD)"
npm install --unsafe
npm rebuild grpc --runtime=electron --target=7.3.0
chmod +x protoc.sh && ./protoc.sh $utbot_dir/server/proto $utbot_dir/vscode-plugin/src/proto-ts
npm run compile
