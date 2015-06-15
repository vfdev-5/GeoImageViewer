/*!
*
* Geo Image Viewer project
*
* Shaper application : GraphicsScene + draw rectangles
*
*/


// Qt
#include <QApplication>

// Project
#include "MainWindow.h"


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();

}
