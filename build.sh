#!/bin/bash

mkdir cmake_build
pushd cmake_build

cmake ../server
make

popd
cp cmake_build/fserve .
rm -rf cmake_build
