#!/bin/bash

set -e

if (( $EUID != 0 )); then
    echo "Please run as root"
    exit 1
fi

# go to the dir of this script
cd "$(dirname "$0")"


# necessary to compile
apt-get install make cmake cmake-data pkg-config build-essential -y


function ensure_git {
    if ! [ -x "$(command -v git)" ]; then
        echo "Installing git..."
        apt-get install git -y
    fi
}


# download, compile, and install gRPC
# we detect the presence of gRPC using its protoc plugin
if ! [ -x "$(command -v grpc_cpp_plugin)" ]; then

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

    echo "Installing gRPC..."

    # other build tools
    apt-get install -y build-essential autoconf libtool pkg-config

    # clone repo
    ensure_git
    git clone --recurse-submodules -b v1.28.1 https://github.com/grpc/grpc
    cd grpc

    mkdir -p cmake/build
    pushd cmake/build
    ./../../../cmake-3.17.0/bin/cmake -DgRPC_INSTALL=ON \
        -DgRPC_BUILD_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr \
        ../..
    make -j
    make install
    popd

    cd ..

else

    echo "gRPC is already installed."

fi
