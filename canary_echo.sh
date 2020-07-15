#!/usr/bin/env bash

ORANGE='\033[0;33m'
NC='\033[0m' # No Color
CANARY_PROJECT="${ORANGE}[CanaryProject]${NC}"

echo -e "${CANARY_PROJECT} $@"