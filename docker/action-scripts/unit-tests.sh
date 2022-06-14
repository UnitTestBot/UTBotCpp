#!/bin/bash

source docker/building_dependencies/runtime_env.sh
cd server/build
chmod +x UTBot_UnitTests
./UTBot_UnitTests --verbosity info --log `pwd`
