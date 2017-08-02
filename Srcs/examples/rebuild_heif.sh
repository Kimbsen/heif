#!/bin/sh

cd ../../
cmake .
#make clean
make
cd Srcs/examples/
cp ../*/*.a .
