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


# download, compile, and install Antlr4
# we detect the presence of Antlr using its lib*.so file
if [ ! -f /usr/lib/libantlr4-runtime.so.4.7.1 ]; then

    echo "Installing Antlr 4.7.1"

    echo "Clean up"

    rm -rf antlr4
    mkdir antlr4
    cd antlr4

    echo "Download release"

    apt-get install unzip wget -y
    wget -O antlr-4.7.1-complete.jar 'http://www.antlr.org/download/antlr-4.7.1-complete.jar'
    wget -O antlr-4.7.1.zip 'https://github.com/antlr/antlr4/archive/4.7.1.zip'
    unzip -q antlr-4.7.1.zip


    echo "Build Antlr4"


    # install Antlr4 dependency
    apt-get install uuid-dev cmake make -y

    # clean up an prepare build
    cd antlr4-4.7.1/runtime/Cpp
    rm -rf dist
    rm -rf build && mkdir build
    rm -rf run && mkdir run

    # build
    cd build
    cmake .. -DANTLR_JAR_LOCATION=../../../../antlr4.7.1-complete.jar
    make
    DESTDIR=../run make install
    cd ..

    echo "Install Antlr4 files"

    cp -r run/usr/local/include/antlr4-runtime /usr/include
    cp -r run/usr/local/lib/* /usr/lib/

    cd ../../.. # antlr4-4.7.1/runtime/Cpp

    cd .. # antlr4

    echo "Done installing Antlr 4.7.1"

else

    echo "Antlr 4.7.1 is already installed."

fi
