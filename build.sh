#!/bin/bash
set -e
set -o pipefail
pwd=$PWD
chmod +x $pwd/submodules/build-klee.sh $pwd/submodules/Bear/build.sh $pwd/server/build.sh
cd $pwd/submodules && ./build-klee.sh
cd $pwd/submodules/Bear && ./build.sh
if [ "$1" = "gh" ]; then
  rm -rf submodules
fi
cd $pwd/server && ./build.sh
