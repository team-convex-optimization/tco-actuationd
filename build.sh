#!/bin/bash

mkdir -p build
pushd build

clang \
    -Wall \
    -std=c11 \
    -D _BSD_SOURCE \
    -I ..\
    -I /usr/include/linux/i2c-dev.h \
    ../code/pca9685.c \
    ../code/main.c \
    ../code/io.c \
    -o tco-server-io.bin \
    -O \
    
popd
