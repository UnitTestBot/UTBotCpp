#!/bin/bash

#install java zulu 11 distribution
sudo apt update -y
sudo apt install dirmngr --install-recommends -y
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 0xB1998361219BD9C9
sudo apt-add-repository 'deb http://repos.azulsystems.com/ubuntu stable main' -y

sudo apt update -y
sudo apt install zulu-11 -y


set -e
cd clion-plugin
./gradlew assemble
