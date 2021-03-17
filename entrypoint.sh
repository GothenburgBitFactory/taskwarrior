#!/bin/bash

git submodule init
git submodule update
cmake -DCMAKE_BUILD_TYPE=debug .
make -j2
make install
task --version

cd test || exit
make
./run_all -v
./problems
make clean

cd ../
make clean
