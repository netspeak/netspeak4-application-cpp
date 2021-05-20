# Env

The idea behind this Docker image is that it contains all global dependencies necessary to build the C++ application.

The main reason for this it to shorten build times. The C++ application has some pretty heavy dependencies that have to be compiled, so doing this beforehand saves a lot of time.

To build the image, run:

```bash
docker build .
```
(May require `sudo`.)

The prebuilt image is `webis/netspeak-env:latest`.
