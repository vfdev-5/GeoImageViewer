#!/bin/bash

## Script to generate QtCreator project

QT_CREATOR=/opt/Qt5.4.0/Tools/QtCreator/bin/qtcreator

export QT_DIR=/opt/Qt5.4.0/5.4/gcc_64/
export GDAL_DIR=/opt/source/gdal-trunk/build/
export GDAL_DATA=$GDAL_DIR/share/gdal

gdal_lib=$GDAL_DIR/lib
LD_LIBRARY_PATH=$gdal_lib:/usr/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

$QT_CREATOR CMakeLists.txt

# don't forget to specify : Debug, Release
# -DCMAKE_BUILD_TYPE=Debug
