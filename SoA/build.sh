#!/usr/bin/env bash
if [ "$1" == "clean" ] ; then
    rm -Rf build
fi
if [ ! -d "build" ] ; then
    mkdir build
fi
# clean out CMakeFiles...
rm -Rf cmake CMakeFiles CmakeCache.txt
cd build
cmake ../
if [ "$1" == "clean" ] ; then
    make clean
    shift
fi
make doc
make $@