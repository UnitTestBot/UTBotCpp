#!/bin/bash

chmod +x docker/action-scripts/build-utbot.sh
docker/action-scripts/build-utbot.sh

rm -rf server/build
