#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

#for fail-fast execution mode
set -e
#Path variables
HOME_DIR=/home/utbot/UTBotCpp
VSCODE_VERSION_DIR=$(ls -d -1 $HOME_DIR/vscode-plugin/.vscode-test/vscode-*/ |sed -n '1p')
echo "VSCODE_VERSION_DIR=$VSCODE_VERSION_DIR"

#Starting the X-server
export DISPLAY=':99.0'
Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &

#Executing the test suite
#TODO: fetch workspace folders automatically from .vscode/launch.json
$VSCODE_VERSION_DIR/VSCode-linux-x64/code $HOME_DIR/integration-tests/c-example \
  --extensionDevelopmentPath=$HOME_DIR/vscode-plugin \
  --extensionTestsPath=$HOME_DIR/vscode-plugin/out/test/suite/index \
  --no-sandbox