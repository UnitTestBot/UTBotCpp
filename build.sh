#!/bin/bash
set -e
set -o pipefail
pwd=$PWD
chmod +x $pwd/submodules/klee/build.sh $pwd/submodules/Bear/build.sh $pwd/server/build.sh
cd $pwd/submodules/klee && ./build.sh
cd $pwd/submodules/Bear && ./build.sh
cd $pwd/server && ./build.sh
