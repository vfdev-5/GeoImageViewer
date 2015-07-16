#ifndef FLOATINGDATAPROVIDER_H
#define FLOATINGDATAPROVIDER_H


// Opencv
#include <opencv2/core/core.hpp>

// Project
#include "Global.h"
#include "LibExport.h"
#include "ImageDataProvider.h"

namespace Core
{

//******************************************************************************

class GIV_DLL_EXPORT FloatingDataProvider : public ImageDataProvider
{
    Q_OBJECT
public:
    explicit FloatingDataProvider(QObject *parent = 0);
    virtual cv::Mat getImageData(const QRect & srcPixelExtent=QRect(), int dstPixelWidth=0, int dstPixelHeight=0) const;
    void setImageData(const QPoint & offset, const cv::Mat & data);

    static FloatingDataProvider* createDataProvider(const ImageDataProvider *src, const QRect & pixelExtent);
    static FloatingDataProvider* createDataProvider(const QString & name, const cv::Mat & src, const QRect & intersection=QRect());
    static FloatingDataProvider* createEmptyDataProvider(const QString & name, int width, int height);

    virtual QString fetchProjectionRef() const
    { return _projectionRef; }
//    virtual QPolygonF fetchGeoExtent(const QRect & pixelExtent=QRect()) const;
    virtual QPolygonF fetchGeoExtent(const QVector<QPoint> & points=QVector<QPoint>()) const;
    virtual QVector<double> fetchGeoTransform() const
    { return _geoTransform; }


    void setProjectionRef(const QString & p)
    { _projectionRef = p; }
    void setGeoTransform(const QVector<double> & gt)
    { _geoTransform = gt; }
    void setGeoExtent(const QPolygonF & ge)
    { _geoExtent = ge; }

    bool create(const QString & name, const cv::Mat & src, const QRect & intersection=QRect());
    void setupGeoInfo(const ImageDataProvider * src, const QRect & intersection=QRect());

    virtual bool isValid() const
    { return !_data.empty(); }

signals:
    void dataChanged(const QRect & pixelExtent);


protected:

    cv::Mat _data;

    QString _projectionRef;
    QVector<double> _geoTransform;
    QPolygonF _geoExtent;


};

//******************************************************************************

}

#endif // FLOATINGDATAPROVIDER_H
