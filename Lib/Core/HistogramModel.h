#ifndef HISTOGRAMMODEL_H
#define HISTOGRAMMODEL_H

// Qt
#include <QObject>

// Project
#include "LibExport.h"
#include "ImageDataProvider.h"

namespace Core
{

//*************************************************************************

class GIV_DLL_EXPORT HistogramModel : public QObject
{
    Q_OBJECT

//    struct BandHistogram {
//        double xmin;
//        double xmax;
//        double qxmin;
//        double qxmax;
//        QVector<double> data;
//        QString name;
//    };

public:

    explicit HistogramModel(QObject *parent = 0);
    void setDataProvider(const ImageDataProvider *provider)
    { _data = provider; }

protected:

//    QList<BandHistogram*> _histograms;
    const ImageDataProvider * _data;



};

//*************************************************************************

}

#endif // HISTOGRAMMODEL_H
