#!/usr/bin/env bash
TYPE="Debug"
TESTS="Off"
EXECUTE="Off"

usage()
{
    ./canary_echo.sh "Usage: -r for release, -t for build tests and -e for execute]]"
    exit 1
}

while getopts "rteh" opt; do
  case "$opt" in
    r) TYPE="Release" ;;
    t) TESTS="On" ;;
    e) EXECUTE="On" ;;
    h) usage ;;
  esac
done

cd build
cmake -DOPTIONS_ENABLE_UNIT_TEST=${TESTS} -DCMAKE_BUILD_TYPE=${TYPE} .. ; make -j`nproc`
cd ..
rm -rf canary
cp build/bin/canary ./

if [ "$EXECUTE" = "On" ]; then
  ./canary
fi
