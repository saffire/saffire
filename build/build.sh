#!/bin/sh

rm CMakeCache.txt cmake_install.cmake Makefile
rm -rf src CMakeFiles src unittests
cmake -B. -D CMAKE_BUILD_TYPE=Release .. && VERBOSE=1 make
