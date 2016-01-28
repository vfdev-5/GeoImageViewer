GeoImageViewer
==============

A project to simplify the work with geo images. 
Based on Qt QGraphicsScene/View, image is shown in its own geometry. Image reading is done using GDAL.

Project main dependencies:
* Qt ( >= 5)
* GDAL ( >= 1.11, < 2.0)
* OpenCV ( >= 2.4)


Project contains :
* Library ('Lib/') with bricks to create geo-image aware applications 
* Example applications ('App/') showing basic usage of the library
* Tests
* Plugins

## TODO

-
    - Create hierarchical structures on the main image
        -- Dislpay the root QObject and its childrens
        -- React when a child added or removed
        -- Drag and drop 
        -- Customized Context Menu
            --
-
