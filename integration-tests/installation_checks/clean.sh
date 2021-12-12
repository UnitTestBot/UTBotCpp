#!/bin/bash

log (){
        echo [$(date +'%Y-%m-%d %H:%M:%S')] $1
}
WORK_DIR=$(pwd)

cd "${WORK_DIR}" || exit 1
log "Cleanup started"
rm -rf ${WORK_DIR}/utbot_distr
cd ..
log "Cleanup completed"