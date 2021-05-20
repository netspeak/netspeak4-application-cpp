# This is a multi-stage build.
# This is the first stage where the goal is to build the Netspeak binary from source.

FROM webis/netspeak-env:0.1 AS builder

ENV DEBIAN_FRONTEND noninteractive

RUN mkdir /netspeak
WORKDIR /netspeak
RUN apt-get install git -y

# Checkout a specific commit instead of a specific branch.
# This makes builds deterministic.
RUN git clone https://github.com/netspeak/netspeak4-application-cpp.git . \
	&& git checkout 49c440e794e05239f55216a84b835acb0a93b479

RUN bash ./build/install-dependencies.sh ci
RUN bash ./build/cmake.sh
RUN bash ./build/make-release.sh


# Now for the real image.
# `webis/netspeak-env` is based on `debian:buster` and Netspeak compiles into a fat executable,
# so we can just copy it over without worrying about dependencies.

FROM debian:buster

ENV DEBIAN_FRONTEND noninteractive

COPY --from=builder /netspeak/build/release/netspeak4 /usr/local/bin/

# Print the help message by default.
CMD netspeak4 --help
