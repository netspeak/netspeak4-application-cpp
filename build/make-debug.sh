#!/bin/bash

set -e

# go to the dir of this script
cd "$(dirname "$0")"


# run cmake if not done already
if [ ! -d ./debug ]; then
    bash ./cmake.sh
fi

# switch to debug build
cd ./debug

# run make with as many processing core as we have
make -j$(nproc)
