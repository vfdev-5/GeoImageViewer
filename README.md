GeoImageViewer
==============

The project is inteded to simplify a processing of geographical images. Image viewer is based on Qt QGraphicsScene/View. Image is displayed in its own geometry. Image reading is done using GDAL. Image processing is with OpenCV.




### Project main dependencies:
* Qt ( >= 5)
* GDAL ( == 1.11)
* OpenCV ( >= 2.4)


Project contains :
* Library ('Lib/') with bricks to create geo-image aware applications
* Example application ('App/') showing basic usage of the library
* Tests
* Plugins

### Installation :

```
$ git clone https://github.com/vfdev-5/GeoImageViewer GIV_source
$ mkdir build_GIV; cd build_GIV
$ cmake -DCMAKE_INSTALL_PREFIX=../GeoImageViewer ../GIV_source/
$ make -j4 install
```


### Start demo application
It is necessary that the executable application could find the libraries in `GeoImageViewer/lib`.
```
$ export LD_LIBRARY_PATH=$PWD/GeoImageViewer/lib:$LD_LIBRARY_PATH
$ ./GeoImageViewer/bin/GeoImageViewerApp
```
