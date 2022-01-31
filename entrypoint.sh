#!/bin/bash
set -euo pipefail

cp -r /root/src /root/code
cd /root/code

git submodule init
git submodule update
if [ -f CMakeCache.txt ]; then
  rm CMakeCache.txt
fi
cmake -DCMAKE_BUILD_TYPE=debug .
make -j2
make install
task --version

cd test || exit
make
./run_all -v
./problems
