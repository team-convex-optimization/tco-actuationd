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
    -I /usr/include \
    -I ../lib/tco_shmem \
    -I ../lib/tco_libd/include \
    -l pthread \
    -l rt \
    -l i2c \
    -l ncurses \
    -l gpiod \
    ../code/*.c \
    tco_libd.a \
    -o tco_actuationd.bin \
    -O 
popd
