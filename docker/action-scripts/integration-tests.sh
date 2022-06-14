#!/bin/bash

source docker/building_dependencies/runtime_env.sh
cd vscode-plugin

#download vs_code
npm run download_vscode

chmod +x src/test/run.sh
./src/test/run.sh
