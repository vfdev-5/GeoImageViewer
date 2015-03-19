#ifndef BLURFILTER_H
#define BLURFILTER_H

// Qt
#include <QObject>

// Project
#include "Filters/AbstractFilter.h"

namespace Filters
{

//******************************************************************************

class GIV_DLL_EXPORT BlurFilter : public AbstractFilter
{
    Q_OBJECT

    Q_PROPERTY(int sizeX READ getSizeX WRITE setSizeX)
    PROPERTY_GETACCESSOR(int, sizeX, getSizeX)

    Q_PROPERTY(int sizeY READ getSizeY WRITE setSizeY)
    PROPERTY_GETACCESSOR(int, sizeY, getSizeY)

    Q_CLASSINFO("sizeX","minValue:1;maxValue:50")
    Q_CLASSINFO("sizeY","minValue:1;maxValue:50")

public:
    BlurFilter(QObject * parent = 0);
    virtual ~BlurFilter() {}

    void setSizeX(int v)
    { _sizeX = v; }

    void setSizeY(int v)
    { _sizeY = v; }


protected:
    virtual cv::Mat filter(const cv::Mat & src) const;

};

//******************************************************************************

}

#endif // AbstractFilter_H
