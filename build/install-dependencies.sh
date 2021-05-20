#!/bin/bash

set -e

if (( $EUID != 0 )); then
    echo "Please run as root"
    exit 1
fi

# go to the dir of this script
cd "$(dirname "$0")"

apt-get update
apt-get install make clang build-essential libboost-filesystem-dev libboost-system-dev libboost-test-dev libboost-program-options-dev libboost-stacktrace-dev libcmph-dev libaio-dev libboost-regex-dev libboost-date-time-dev -y


bash ./env/install-antlr4.sh
bash ./env/install-grpc.sh


mkdir -p ./dependencies
chmod a+rw -R ./dependencies
cd ./dependencies

if [[ "$1" != "ci" ]]; then

    # Download the JAR necessary to copmile .g4 files
    if [ ! -f ./antlr4/antlr-4.7.1-complete.jar ]; then

        echo "Downloading Antlr 4.7.1 JAR."

        mkdir -p antlr4
        cd antlr4

        apt-get install unzip wget -y
        wget -O antlr-4.7.1-complete.jar 'http://www.antlr.org/download/antlr-4.7.1-complete.jar'
        chmod a+rw ./antlr-4.7.1-complete.jar

        cd ..

    else

        echo "Antlr 4.7.1 JAR already present"

    fi


    # Download protoc-gen-grpc-web, a protoc plugin to generate gRPC web code
    if [ ! -f ./protoc-gen-grpc-web ]; then

        wget -O protoc-gen-grpc-web 'https://github.com/grpc/grpc-web/releases/download/1.2.0/protoc-gen-grpc-web-1.2.0-linux-x86_64'
        chmod a+rwx ./protoc-gen-grpc-web

    fi

    # Download protoc-gen-grpc-java, a protoc plugin to generate gRPC Java code
    if [ ! -f ./protoc-gen-grpc-java ]; then

        wget -O protoc-gen-grpc-java 'https://repo1.maven.org/maven2/io/grpc/protoc-gen-grpc-java/1.32.2/protoc-gen-grpc-java-1.32.2-linux-x86_64.exe'
        chmod a+rwx ./protoc-gen-grpc-java

    fi

fi
