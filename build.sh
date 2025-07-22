#!/bin/bash

set -e

if [ -d build ]; then
  echo "Removing existing build directory..."
  rm -rf build
fi

mkdir -p build
cd build
echo "Configuring with CMake..."
cmake -G "Unix Makefiles" ..
echo "Building the project..."
make
