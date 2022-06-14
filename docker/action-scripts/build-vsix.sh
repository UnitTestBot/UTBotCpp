#!/bin/bash

source docker/building_dependencies/runtime_env.sh
cd vscode-plugin
echo $VERSION
npm version $VERSION --allow-same-version
utbot_dir="$(dirname $PWD)"
npm install -g vsce --unsafe
npm install --unsafe
npm rebuild grpc --runtime=electron --target=7.3.0
chmod +x protoc.sh && ./protoc.sh $utbot_dir/server/proto $utbot_dir/vscode-plugin/src/proto-ts
npm run compile
vsce package --no-yarn
