/*!
*
* Geo Image Viewer project
*
* An application for displaying satellite images and using image processing procedures
*
* Based on Qt QGraphicsScene/View, image is shown in its own geometry. Image reading is done using GDAL.
*
*
*/


// Qt
#include <QApplication>

#include <QLabel>

// Project
#include "MainWindow.h"
#include "Tools/ToolsManager.h"
#include "Filters/FiltersManager.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    //     Setup plugins
//    Tools::ToolsManager::get()->loadPlugins(qApp->applicationDirPath() + "/Plugins/Tools");
    Filters::FiltersManager::get()->loadPlugins(qApp->applicationDirPath() + "/Plugins/Filters");
    //     display main window
    MainWindow w;
    w.show();
    return a.exec();

}
