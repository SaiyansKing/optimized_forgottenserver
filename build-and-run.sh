#!/usr/bin/env bash

cd build
sudo cmake -DPACKAGE_TESTS=Off .. ; make
cd ..
rm -rf canary
cp build/canary ./
./canary 