#!/bin/bash

set -e

cd clion-plugin
./gradlew assemble
rm -rf /root/.gradle

