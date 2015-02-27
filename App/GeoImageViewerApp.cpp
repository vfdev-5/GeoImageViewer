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
* 2) Image loading/display
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
*   + layers view
*   - display image info
*
*/


// Qt
#include <QApplication>

#include <QLabel>

// Project
#include "MainWindow.h"
#include "Tools/ToolsManager.h"

// TEST
#include <iostream>
#include <limits>
#include <opencv2/core/core.hpp>
#include <Core/LayerUtils.h>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
// TEST

// ANOTHER TEST
#include "Core/ImageOpener.h"
#include "Core/ImageDataProvider.h"
#include "Core/ImageRenderer.h"
#include <QPushButton>
#include <QImage>
// ANOTHER TEST

int main(int argc, char *argv[])
{

/*
    // ANOTHER TEST
    QApplication a(argc, argv);

    // To GraphicsScene:
    QWidget w;
    w.setLayout(new QVBoxLayout());
    QGraphicsScene scene;
    QGraphicsView view(&w);
    w.layout()->addWidget(&view);


    view.setScene(&scene);

    QGraphicsPixmapItem * item = 0;
    Core::ImageDataProvider * provider = 0;

    QPushButton * btn0 = new QPushButton("add", &w);
    w.layout()->addWidget(btn0);
    QObject::connect(btn0, &QPushButton::clicked,
        [&] ()
    {
        QString ifile = "file:///C:/VFomin_folder/PISE_project/Images/COSMO-SkyMed/Test_Sublooking_HP/dgm/CSKS2_DGM_B_HI_24_VV_RD_SF_20130517205139_20130517205146.h5";
        Core::ImageOpener io;
        Core::ImageRenderer renderer;
        provider = io.openImage(QUrl(ifile));
        if (!provider)
            return 1;
        renderer.setupConfiguration(provider);
        QRect ext = provider->getPixelExtent();
        ext.adjust(-100,-100,100,100);

        {
            QPixmap p;
            cv::Mat r;
            {
                cv::Mat data = provider->getImageData(ext, 2000);
                //            Core::displayMat(data, true, "data");
                r = renderer.render(data);
                //            Core::displayMat(r, true, "r");
            }

            p = QPixmap::fromImage(QImage(r.data,
                                          r.cols,
                                          r.rows,
                                          QImage::Format_ARGB32).copy());

            item = new QGraphicsPixmapItem(p);
            item->setTransform(
                        QTransform::fromScale(0.33, 0.33) *
                        QTransform::fromTranslate(+1.2, 0.5)
                        );
        }

        scene.addItem(item);

    });


    QPushButton * btn = new QPushButton("clear", &w);
    w.layout()->addWidget(btn);
    QObject::connect(btn, &QPushButton::clicked,
        [&] ()
    {
        scene.clear();

        if (provider) delete provider;
    });

    w.show();


    // GDAL MEM TEST

//    GDALAllRegister();
//    //QString filepath = "C:/VFomin_folder/PISE_project/Images/COSMO-SkyMed/1_ScanSar_HUGE/L1B/demos/CSKS1_DGM_B_HR_00_HH_RA_SF_20080211191721_20080211191753.h5";
//    QString filepath = "/home/vfomin/pise_project/Images/CSK/CSKS1_DGM_B_HR_00_HH_RA_SF_20080211191721_20080211191753.h5";
////    QString filepath = "C:/VFomin_folder/PISE_project/Images/COSMO-SkyMed/Test_Sublooking_HP/dgm/CSKS2_DGM_B_HI_24_VV_RD_SF_20130517205139_20130517205146.h5";
//    GDALDataset* dataset = static_cast<GDALDataset *>(GDALOpen( filepath.toStdString().c_str(), GA_ReadOnly ));
//    if( !dataset )
//    {
//        return 1;
//    }

//    GDALClose( dataset );
//    GDALDestroyDriverManager();

//    int aa = 1;
//    aa++;


    // GDAL MEM TEST

    return a.exec();
    // ANOTHER TEST

*/


    QApplication a(argc, argv);
    //     Setup plugins
    Tools::ToolsManager::get()->loadPlugins(qApp->applicationDirPath() + "/Plugins");
    //     display main window
    MainWindow w;
    w.show();
    return a.exec();

}
