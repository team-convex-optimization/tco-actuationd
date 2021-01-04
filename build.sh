#!/bin/bash

mkdir -p build

pushd lib/tco_libd
./build.sh
mv -f build/tco_libd.a ../../build
popd

pushd build
clang \
    -Wall \
    -std=c11 \
    -D _DEFAULT_SOURCE \
    -I ..\
    -l i2c \
    -I /usr/include/linux/i2c-dev.h \
    -I ../lib/tco_shmem \
    -I ../lib/tco_libd/include \
    -lpthread \
    -lrt \
    ../code/pca9685.c \
    ../code/main.c \
    ../code/actuator.c \
    tco_libd.a \
    -o tco-actuationd.bin \
    -O 
popd
