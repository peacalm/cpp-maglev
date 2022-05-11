#!/usr/bin/env bash

cd test/unit_test
mkdir -p build
cd build
cmake .. -DENABLE_MYOSTREAM_WATCH=TRUE
make
./unit_test


cd ../../performance_test
mkdir -p build
cd build
cmake .. -DENABLE_MYOSTREAM_WATCH=TRUE
make
./performance_test

