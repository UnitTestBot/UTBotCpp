#!/bin/bash

set -e

if [ "$#" -ne 1 ]; then
        echo "Illegal number of parameters, exactly one is expected (build version)"
        exit 1;
fi

# Script constants
TARGET_ARCHIVE=$1
WORK_DIR=$(pwd)
EXPECTED_PID_FILE_LOCATION=${WORK_DIR}/utbot_distr/server-install/utbot.pid
OS_DESCRIPTION_FILE=/etc/os-release
declare -a SUPPORTED_DISTRIBUTIONS=("ubuntu")
declare -a SUPPORTED_UBUNTU_RELEASES=("20.04")

# COMMON FUNCTIONS:
log (){
        echo [$(date +'%Y-%m-%d %H:%M:%S')] $1
}

check_if_file_exists(){
  if [ ! -f "$1" ]
  then
    log "ERROR: Required file [$1] is not present in the file system"
    exit 1
  fi
}

extract_value_for_property(){
  FILE=$1
  PROPERTY_NAME=$2

  cat "$FILE" | grep -E "^$PROPERTY_NAME=" | cut -d"=" -f2
}

contains(){
  TBF=$1
  POINTER=$2[@]
  SET=("${!POINTER}")

  for ver in "${SET[@]}"; do
    if [ "$ver" == "$TBF" ] ; then
       return 0
    fi
  done

  log "$3"
  exit 1
}

check_os(){
  log "OS Version Checks - started"
  check_if_file_exists "${OS_DESCRIPTION_FILE}"

  DISTRO_NAME=$( extract_value_for_property ${OS_DESCRIPTION_FILE} "ID" )
  VERSION_ID=$( extract_value_for_property ${OS_DESCRIPTION_FILE} "VERSION_ID" | sed -e 's/"//g' )

  contains $DISTRO_NAME SUPPORTED_DISTRIBUTIONS "[$DISTRO_NAME] is not found in the supported distributions list"
  contains $VERSION_ID SUPPORTED_UBUNTU_RELEASES "[$VERSION_ID] is not found in the supported versions list for OS [$DISTRO_NAME]"
  log "OS Version Checks - completed"
}

check_if_file_executable(){
  if [ ! -x "$1" ]
  then
    log "ERROR: File [$1] is not executable"
  fi
}

kill_process(){
  log "killing process by pid [$1]"
  kill "$1"
}

check_if_server_process_started(){
   check_if_file_exists $EXPECTED_PID_FILE_LOCATION
   PID=$(cat $EXPECTED_PID_FILE_LOCATION)
   process_count=$(ps -p "$PID" --no-header | wc -l)

   log "Number of processes found by PID [$PID] : $[process_count]"

   if [ $process_count -ge 1 ]
   then
      kill_process "$PID"
   else
      log "ERROR: no processes found by PID [$PID]"
      exit 1
   fi
}

unarchive_distro(){
  log "Running unpacking - started"
  check_if_file_exists     ${WORK_DIR}/utbot_distr.tar.gz
  check_if_file_exists     ${WORK_DIR}/unpack_and_run_utbot.sh
  check_if_file_executable ${WORK_DIR}/unpack_and_run_utbot.sh

  log "Running unpacking - completed"
}

run_installation(){
    log "Installation Script Execution - started"
    cd ${WORK_DIR}
    ./unpack_and_run_utbot.sh > /dev/null 2>&1

    script_exit_code=$?
    log "Script exit code is [$script_exit_code]"
    if [ "$script_exit_code" -ne 0 ]; then
       log "ERROR: Installation script return non-zero exit code"
       exit 1;
    fi
    cd ..
    log "Installation Script Execution - completed"
}

# TEST EXECUTION:
log "Smoke test on installation routine for UTBotCpp ${TARGET_ARCHIVE} - started"

check_os
unarchive_distro
run_installation
check_if_server_process_started

log "Smoke test on installation routine for UTBotCpp ${TARGET_ARCHIVE} - completed successfully"
