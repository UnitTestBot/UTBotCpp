#!/bin/bash

# enable needed envs for server
source docker/building_dependencies/runtime_env.sh

sudo apt-get update -y

# install font config. Without it java.awt will throw, and ide will exit.
apt-get install libfreetype6 fontconfig fonts-dejavu -y

#install java zulu 11 distribution
sudo apt update -y
sudo apt install dirmngr --install-recommends -y
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 0xB1998361219BD9C9
sudo apt-add-repository 'deb http://repos.azulsystems.com/ubuntu stable main' -y

sudo apt update -y
sudo apt install zulu-11 -y


set -e

./server/build/utbot server > /dev/null 2>&1 &

cd clion_plugin
./gradlew test --info --rerun-tasks


