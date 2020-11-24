#!/bin/bash

set -e

if (( $EUID != 0 )); then
    echo "Please run as root"
    exit 1
fi

# go to the dir of this script
cd "$(dirname "$0")"


mkdir -p ./dependencies
cd ./dependencies

# Install a recent CMake version
if [ ! -d ./cmake-3.17.0 ]; then

    # the default version of CMake under most distributions is too low

    # This will install a local version of cmake.
    # This does not affect the cmake version installed on your system

    echo "Installing CMAKE 3.17.0..."

    mkdir -p cmake-3.17.0
    cd cmake-3.17.0

    apt-get install -y wget
    wget -q -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0-Linux-x86_64.sh
    sh cmake-linux.sh -- --skip-license --prefix=./
    rm cmake-linux.sh

    cd ..
fi

cd ..


# Configure CMake
cd ..
./build/dependencies/cmake-3.17.0/bin/cmake .