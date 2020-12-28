#!/bin/bash

mkdir -p build
pushd build

clang \
    -Wall \
    -std=c11 \
    -D _DEFAULT_SOURCE \
    -I ..\
    -I /usr/include/linux/i2c-dev.h \
    -I ../lib/tco_shmem \
    -lpthread \
    -lrt \
    ../code/pca9685.c \
    ../code/main.c \
    ../code/io.c \
    -o tco-server-io.bin \
    -O \
    
popd
