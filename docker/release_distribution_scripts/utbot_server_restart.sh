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

#Prompt kill all running UTBot servers with same port
IFS=";"
RUNNING=$( ps aux | grep "utbot server" | grep -v grep | grep "\-\-port $UTBOT_PORT" )

if test -n "$RUNNING"
then
    count=$(echo -n "$RUNNING" | grep -c '^')
    echo "Now RUNNING $count instance(s) of UTBot server with same port:"
    ps aux | head -n 1
    for pp in $RUNNING; do
        echo $pp
    done
    read -r -p "Do you want kill them? [Y/n] " response
    if [[ $response =~ ^(yes|y| ) ]] || [[ -z $response ]]
    then
        for pp in $RUNNING; do
            echo $pp | awk '{print $2;}' | xargs kill
        done
    fi
fi


# Identify the directory where the current script is located
export CURRENT_FOLDER="$( cd $( dirname ${BASH_SOURCE[0]} ) && pwd )"
# Get full path a script that launches UTBot
RUN_SYSTEM_SCRIPT_PATH=$CURRENT_FOLDER/utbot_run_system.sh
# Start script that launches UTBot
$RUN_SYSTEM_SCRIPT_PATH "server" $UTBOT_PORT