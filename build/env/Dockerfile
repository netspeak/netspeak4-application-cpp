FROM debian:buster

ENV DEBIAN_FRONTEND noninteractive

# Make a nice cozy directory
RUN mkdir -p /var/dependencies/
WORKDIR /var/dependencies/

# Install Antlr4
COPY ./install-antlr4.sh ./
RUN apt-get update \
    && bash install-antlr4.sh \
    && rm -rf /var/dependencies/*

# Install gRPC
COPY ./install-grpc.sh ./
RUN apt-get update \
    && bash install-grpc.sh \
    && rm -rf /var/dependencies/*
