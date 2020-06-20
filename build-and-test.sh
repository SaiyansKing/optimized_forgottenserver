#!/usr/bin/env bash

# sudo docker-compose down --rmi all -v --remove-orphans
# sudo docker-compose up --build -d
cd build
sudo cmake -DPACKAGE_TESTS=On .. ; make -j`nproc`
cp tests/canary_tests ../tests/src
cd ../tests/src
./canary_tests --reporter compact --success -d yes
cd ../..
# sudo docker-compose down --rmi all -v --remove-orphans 