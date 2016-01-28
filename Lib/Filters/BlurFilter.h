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

    Q_PROPERTY(QString type READ getType WRITE setType)
    PROPERTY_GETACCESSOR(QString, type, getType)
    Q_CLASSINFO("type","possibleValues:mean,median,gaussian")

    Q_PROPERTY(int sizeX READ getSizeX WRITE setSizeX)
    PROPERTY_GETACCESSOR(int, sizeX, getSizeX)
    Q_CLASSINFO("sizeX","minValue:1;maxValue:100")

    Q_PROPERTY(int sizeY READ getSizeY WRITE setSizeY)
    PROPERTY_GETACCESSOR(int, sizeY, getSizeY)
    Q_CLASSINFO("sizeY","minValue:1;maxValue:100")

    Q_PROPERTY_WITH_ACCESSORS(double, sigmaX, getSigmaX, setSigmaX)
    Q_CLASSINFO("sigmaX","label:Sigma X for gaussian blur;minValue:0.001;maxValue:500.0")

    Q_PROPERTY_WITH_ACCESSORS(double, sigmaY, getSigmaY, setSigmaY)
    Q_CLASSINFO("sigmaY","label:Sigma Y for gaussian blur;minValue:0.001;maxValue:500.0")



public:
    BlurFilter(QObject * parent = 0);
    virtual ~BlurFilter() {}

    void setSizeX(int v)
    { _sizeX = v; }

    void setSizeY(int v)
    { _sizeY = v; }

    void setType(const QString & type)
    {
        _type=type;
    }

protected:
//    void setName()
//    {
//        _name = tr("Blur filter : %1").arg(_type);
//    }

    virtual cv::Mat filter(const cv::Mat & src) const;

};

//******************************************************************************

}

#endif // BLURFILTER_H
