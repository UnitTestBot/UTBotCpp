#!/bin/bash

# enable needed envs for server
source docker/building_dependencies/runtime_env.sh

sudo apt-get update -y

# install font config. Without it java.awt will throw, and ide will exit.
apt-get install libfreetype6 fontconfig fonts-dejavu -y

set -e

$UTBOT_ALL/server-install/utbot server > /dev/null 2>&1 &

cd clion-plugin
./gradlew test --info --rerun-tasks


