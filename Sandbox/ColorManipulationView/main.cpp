
// Qt
#include <QApplication>
#include <qmath.h>

// Project
#include "Core/Global.h"
#include "Gui/ColorManipulationView.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    Gui::ColorManipulationView cmview;

//    cmview.clear();

#if 1
    // GRAY MODE HISTOGRAM
    QGradientStops stops = QGradientStops()
            << QGradientStop(0.1, QColor(Qt::black))
            << QGradientStop(0.3, QColor(Qt::blue))
            << QGradientStop(0.4, QColor(Qt::blue))
            << QGradientStop(0.5, QColor(Qt::blue))
            << QGradientStop(0.6, QColor(Qt::red))
            << QGradientStop(0.7, QColor(Qt::red))
            << QGradientStop(0.9, QColor(Qt::green));

    QVector<double> data(200);
    double xmin = -150.5;
    double xmax = 243.7;

    double step = (xmax - xmin)/(200.0);
    for (int i=0;i<data.size();i++)
    {
        double x = xmin + step * i;
        data[i] = qExp( -x*x / (50.4) );
    }

//    cmview.addHistogram(stops, data, xmin, xmax);
    cmview.addHistogram(stops, data, 0.0, 1.0);
    cmview.drawSingleHistogram(0);

#endif


    cmview.show();

    return a.exec();

}
