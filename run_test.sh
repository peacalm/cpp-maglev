#!/usr/bin/env bash

cd test/unit_test
mkdir -p build
cd build
cmake ..
make
./unit_test

