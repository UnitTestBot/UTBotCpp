#!/bin/bash

source docker/building_dependencies/runtime_env.sh
cd vscode-plugin

service dbus start
#fix error with dri3 on github runner
export LIBGL_DRI3_DISABLE=1

#download vs_code
npm run download_vscode

chmod +x src/test/run.sh
./src/test/run.sh
