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

    static FloatingDataProvider* createDataProvider(const ImageDataProvider *src, const QRect & pixelExtent);
    static FloatingDataProvider* createDataProvider(const QString & name, const cv::Mat & src, const QRect & intersection=QRect());

    virtual QString fetchProjectionRef() const
    { return (_source) ? _source->fetchProjectionRef() :
                         ImageDataProvider::fetchProjectionRef(); }
    virtual QPolygonF fetchGeoExtent(const QRect & pixelExtent=QRect()) const;

    bool create(const QString & name, const cv::Mat & src, const QRect & intersection=QRect());

protected:

    cv::Mat _data;
    const ImageDataProvider * _source;
    QRect _intersection;


};

//******************************************************************************

}

#endif // FLOATINGDATAPROVIDER_H
