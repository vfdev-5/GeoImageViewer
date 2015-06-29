
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
    cmview.clear();

#if 0
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

    cmview.addHistogram(stops, data, xmin, xmax, false);
//    cmview.addHistogram(stops, data, 0.0, 1.0, false);
    cmview.drawSingleHistogram(0);

#else
    // RGB MODE HISTOGRAM
    QGradientStops rStops = QGradientStops()
            << QGradientStop(0.1, QColor(Qt::black))
            << QGradientStop(0.9, QColor(Qt::red));

    QGradientStops gStops = QGradientStops()
            << QGradientStop(0.12, QColor(Qt::black))
            << QGradientStop(0.91, QColor(Qt::green));

    QGradientStops bStops = QGradientStops()
            << QGradientStop(0.2, QColor(Qt::black))
            << QGradientStop(0.7, QColor(Qt::blue));


    QVector<double> rData(200), gData(200), bData(200);
    double xmin = 0;
    double xmax = 1000;

    double step = (xmax - xmin)/(200.0);
    for (int i=0;i<rData.size();i++)
    {
        double x = xmin + step * i;
        rData[i] = qExp( -(x-200.)*(x-200.) / (250.4) );
        gData[i] = qExp( -(x-300.)*(x-300.) / (350.4) );
        bData[i] = qExp( -(x-800.)*(x-800.) / (280.4) );
    }
    cmview.addHistogram(rStops, rData, 0.0, 1.0, false);
    cmview.addHistogram(gStops, gData, 0.0, 1.0, false);
    cmview.addHistogram(bStops, bData, 0.0, 1.0, false);
    cmview.drawRgbHistogram();

#endif


    cmview.show();

    return a.exec();

}
