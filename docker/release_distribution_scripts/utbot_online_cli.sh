#!/bin/bash

#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

# This script launches cli for UTBot online
#arguments: (generate | run) (path to project) (path to snippet)

if [ "$#" -ne 3 ] || { [ "$1" != "generate" ] && [ "$1" != "run" ]; };
then
    echo "Illegal number of parameters. Check: (generate | run) (path-project) (path-snippet)"
    exit 1;
fi

# Identify the directory where the current script is located
export CURRENT_FOLDER="$( cd $( dirname ${BASH_SOURCE[0]} ) && pwd )"
# Get full path a script that launches UTBot
RUN_SYSTEM_SCRIPT_PATH=$CURRENT_FOLDER/utbot_run_system.sh
UTBOT_CLI_OPTIONS="$1 --project-path $2 file --file-path $3"
# Launching cli
$RUN_SYSTEM_SCRIPT_PATH "cli" $UTBOT_CLI_OPTIONS