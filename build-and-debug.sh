#!/usr/bin/env bash

cd build
sudo cmake -DOPTIONS_ENABLE_UNIT_TEST=Off -DCMAKE_BUILD_TYPE=Debug .. ; make
cd ..
rm -rf canary
cp build/bin/canary ./
./canary 