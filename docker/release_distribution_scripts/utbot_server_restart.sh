#!/bin/bash

#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

# This script starts UTBot server

if [ -z "$1" ]
  then
    export UTBOT_PORT=2121
else
    export UTBOT_PORT=$1
fi

# Identify the directory where the current script is located
export CURRENT_FOLDER="$( cd $( dirname ${BASH_SOURCE[0]} ) && pwd )"
# Get full path a script that launches UTBot
RUN_SYSTEM_SCRIPT_PATH=$CURRENT_FOLDER/utbot_run_system.sh
# Start script that launches UTBot
$RUN_SYSTEM_SCRIPT_PATH "server" $UTBOT_PORT