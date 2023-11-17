#!/bin/bash

set -e

cd clion-plugin
./gradlew --no-daemon buildPlugin
rm -rf /root/.gradle
