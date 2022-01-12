#!/bin/bash

#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

# The script provides bash functions that are commonly used in other scripts

# Common functions

# Returns current timestamp
now(){
  echo $(date -d "today" +"%Y%m%d%H%M%S")
}

# Function arguments:
#   - $1 - log message to print out
log(){
  echo [$(date '+%Y-%m-%d %H:%M:%S')] $1
}

# Function arguments:
#   - $1 - process pattern (to print it out in logs only)
#   - $2 - path to program to be started
#   - $3 - process options
#   - $4 - log file
#   - $5 - PID file to be created
start_process(){
  log "INFO Starting new [$1] process right now"
  nohup $2 $3 >> $4 2>&1 &
  local PROCESS_PID=$!
  echo $PROCESS_PID > $5
  log "INFO New [$1] instance with pid [$PROCESS_PID] has been started, process options are [$3], pid file created: [$5]; STDOUT and STDERR redirected to [$4]"
}

# Function arguments:
#   - $1 - process pattern (to print it out in logs only)
#   - $2 - PID
#   - $3 - PID File
kill_process(){
  log "INFO killing [$1] process with pid [$2]"
  kill $2
  if [ -f $3 ]; then
    log "INFO Removing the pid file: [$3]"
    rm $3
  fi
}

# Function arguments:
#   - $1 - file the message should be append on
#   - $2 - log message itself
append_to_file(){
  echo $2 >> $1
}

# Exporting the functions
export -f log
export -f start_process
export -f kill_process
export -f append_to_file
export -f now