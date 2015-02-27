/*!
*
* Geo Image Viewer project
*
* An application for displaying satellite images and using image processing procedures
*
* Based on Qt QGraphicsScene/View, image is shown in its own geometry. Image reading is done using GDAL.
*
*
*
* TODO :
* 1) HistogramLayerRenderer & HistogramRendererView
*   - problem with "all bands" : switch on/off -> histograms are not correctly shown
*   - possibility to choose Bands to RGB mapping for multiband images
*   - choice of RGB / GRAY mode for multiband images
* 2) Image loading
*   + create overview file .ovr
*   - support complex imagery  => GDAL does not work correctly with CSK L1A => pending
*   + support subdataset imagery (netcdf,hdf5)
*   + Message error
* 3) Rendering stage
*   + measure time for loading/rendering a tile => at Release rendering time is correct
* 4) Zoom widget
*   - add zoom buttons & slider
* 5) Tools
*   + Menu with tool buttons
*   - Possibility to draw/edit/select vector layers: point,line,polygon
*   + Possibility to select a zone
* 6) Layer browser
*   -
*
*/


// Qt
#include <QApplication>

#include <QLabel>

// Project
#include "MainWindow.h"


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();

}
