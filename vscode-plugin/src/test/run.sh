#!/bin/bash

#for fail-fast execution mode
set -e

# Identify the directory where the current script is located
CURRENT_FOLDER="$( cd $( dirname ${BASH_SOURCE[0]} ) && pwd )"
PROJECT_DIR=$CURRENT_FOLDER/../../..

VSCODE_VERSION_DIR=$(ls -d -1 $PROJECT_DIR/vscode-plugin/.vscode-test/vscode-* | sed -n '1p')
echo "VSCODE_VERSION_DIR=$VSCODE_VERSION_DIR"

#Starting the X-server
export DISPLAY=':99.0'
Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &

#Executing the test suite
#TODO: fetch workspace folders automatically from .vscode/launch.json
$VSCODE_VERSION_DIR/VSCode-linux-x64/code $PROJECT_DIR/integration-tests/c-example-mini \
  --extensionDevelopmentPath=$PROJECT_DIR/vscode-plugin \
  --extensionTestsPath=$PROJECT_DIR/vscode-plugin/out/test/suite/index \
  --no-sandbox
