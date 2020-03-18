#!/bin/sh
set -e
if [ -n $1 ];
then
    PARAL=$1
else
    PARAL=12
fi

rm -rf ./build
mkdir build
cd build
cmake  -DCMAKE_BUILD_TYPE=Release ../
make -j ${PARAL}
make test
make install/strip
cd ..
