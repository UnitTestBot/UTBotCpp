#!/bin/bash

source docker/building_dependencies/runtime_env.sh
chmod +x build.sh
./build.sh
rm -rf submodules/klee/build
rm -rf server/build
