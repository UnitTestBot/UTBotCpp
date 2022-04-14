#!/bin/bash

#for fail-fast execution mode
set -e

# set environment
source docker/building_dependencies/runtime_env.sh

# Identify the directory where the current script is located
CURRENT_FOLDER="$( cd $( dirname ${BASH_SOURCE[0]} ) && pwd )"
PROJECT_DIR=$CURRENT_FOLDER/../..

# Generate tests
TARGET_PROJECT=$PROJECT_DIR/integration-tests/c-example
mkdir $TARGET_PROJECT/build
cd $TARGET_PROJECT/build
$UTBOT_INSTALL_DIR/bin/cmake ..
rm -f *.json
$UTBOT_ALL/bear/bin/bear make

$UTBOT_ALL/server-install/utbot generate \
--project-path "$TARGET_PROJECT" file \
--file-path "$TARGET_PROJECT/lib/floats/floating_point_plain.c"
