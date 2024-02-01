#!/bin/bash

#for fail-fast execution mode
set -e

sudo apt update && sudo apt install -y build-essential cmake

# install clang
sudo apt update && sudo apt -y install clang-14

#install git for gtest
sudo apt install -y software-properties-common
sudo apt update
sudo add-apt-repository -y ppa:git-core/ppa
sudo apt update
sudo apt install -y git libcurl4-openssl-dev

INSTALL_DIR=/install
UTBOT_CMAKE_BINARY=cmake

sudo git config --global http.sslVerify "false"

#install gtest
sudo git clone --single-branch -b release-1.10.0 https://github.com/google/googletest.git /gtest
cd /gtest && sudo mkdir build && cd build && \
    sudo $UTBOT_CMAKE_BINARY -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR .. && \
    sudo $UTBOT_CMAKE_BINARY --build . --target install && \
    cd /
