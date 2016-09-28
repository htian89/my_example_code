#!/bin/sh
if [ x$1 == "xRelease" ]; then
BUILD_TYPE=Release
else
BUILD_TYPE=Debug
fi

mkdir -p build.$BUILD_TYPE 
cd build.$BUILD_TYPE 
mkdir -p websocket.$BUILD_TYPE vclient.$BUILD_TYPE update_module.$BUILD_TYPE vclientstart_config.$BUILD_TYPE 
cd websocket.$BUILD_TYPE
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../../ || exit 1
make || exit 1
cd -
cd vclient.$BUILD_TYPE
qmake-qt4 QMAKE_CXXFLAGS=-fpermissive ../../vclient || exit 1
make || exit 1
cd -
cd update_module.$BUILD_TYPE
qmake-qt4 QMAKE_CXXFLAGS=-fpermissive ../../vclient/update_module || exit 1
make
cd -
cd vclientstart_config.$BUILD_TYPE
qmake-qt4 QMAKE_CXXFLAGS=-fpermissive ../../vclient/vclientstart_config || exit 1
make || exit 1
cd -
cd ..
