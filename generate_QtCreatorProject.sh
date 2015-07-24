#!/bin/bash

## Script to generate QtCreator project

QT_CREATOR=/opt/Qt5.4.0/Tools/QtCreator/bin/qtcreator

export QT_DIR=/opt/Qt5.4.0/5.4/gcc_64/
#export GDAL_DIR=/usr
#export GDAL_LIBRARY=/usr/lib
#export GDAL_INCLUDE_DIR=/usr/include
#export GDAL_DATA=$GDAL_DIR/share/gdal

qt_lib=$QT_DIR/lib
LD_LIBRARY_PATH=$qt_lib:/usr/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

$QT_CREATOR CMakeLists.txt

# don't forget to specify : Debug, Release
# -DCMAKE_BUILD_TYPE=Debug
