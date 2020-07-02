#!/usr/bin/env bash

# sudo docker-compose down --rmi all -v --remove-orphans
# sudo docker-compose up --build -d
cd build
sudo cmake -DOPTIONS_ENABLE_UNIT_TEST=On .. ; make -j`nproc`
cp bin/canary-tests ../tests/src
cd ../tests/src
./canary-tests --reporter compact --success -d yes
cd ../..
# sudo docker-compose down --rmi all -v --remove-orphans 