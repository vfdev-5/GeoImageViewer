#ifndef FLOATINGDATAPROVIDER_H
#define FLOATINGDATAPROVIDER_H




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

protected:

    cv::Mat _data;


};

//******************************************************************************

}

#endif // FLOATINGDATAPROVIDER_H
