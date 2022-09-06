#!/bin/bash

# enable needed envs for server
source docker/building_dependencies/runtime_env.sh

set -e

cd clion-plugin
./gradlew test --info --rerun-tasks
