#!/bin/bash

set -e

# go to the dir of this script
cd "$(dirname "$0")"

# check protoc version
protocVersion=$(protoc --version)
if [[ ! $protocVersion = "libprotoc 3."* ]]; then
    echo "protoc v3.x.x is required! Your version is $protocVersion";
    exit 1;
fi

cd ../conf

mkdir -p ../src/netspeak/service
rm -rf ../src/netspeak/service/*.{h,cc}
mkdir -p ./generated-js
rm -rf ./generated-js/*
mkdir -p ./generated-java
rm -rf ./generated-java/*

export PATH="$PATH:../build/dependencies"

protoc ./NetspeakService.proto \
    --cpp_out=../src/netspeak/service \
    --grpc_out=../src/netspeak/service --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin)

protoc ./NetspeakService.proto \
    --js_out=import_style=commonjs:./generated-js \
    --grpc-web_out=import_style=typescript,mode=grpcwebtext:./generated-js \

protoc ./NetspeakService.proto \
    --java_out=./generated-java \
    --grpc-java_out=./generated-java
