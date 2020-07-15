#!/usr/bin/env bash

TESTS="ALL"

usage()
{
    ./canary_echo.sh "Usage: [--all, --server, --lib]"
    ./canary_echo.sh "You can pass tests arguments, like -s, --help or -d yes"
    exit 1
}

while getopts ":-:h" opt; do
  case ${opt} in
    -) 
      case ${OPTARG} in
        "all"*) TESTS="ALL"
            ;;
        "lib"*) TESTS="LIB"
            ;;
        "server"*) TESTS="SERVER"
            ;;
      esac 
    ;;
    h) usage ;;
  esac
done

# useful arguments --success -d yes

if [ ${TESTS} != "LIB" ]; then
  ./canary_echo.sh "Running server tests"
  ./build/bin/canary-tests $@
fi

if [ ${TESTS} != "SERVER" ]; then
  ./canary_echo.sh "Running lib tests"
  ./canary-lib/tests/canary-lib-tests $@
fi
