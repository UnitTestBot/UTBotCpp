#!/bin/bash
rm -rf build && mkdir -p build && cd build && cmake .. && /opt/bear/bin/bear make -j8
