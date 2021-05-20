#!/bin/bash

set -e

# go to the dir of this script
cd "$(dirname "$0")"


# run cmake if not done already
if [ ! -d ./release ]; then
    bash ./cmake.sh
fi

# switch to release build
cd ./release

# run make with as many processing core as we have
make -j$(nproc)
