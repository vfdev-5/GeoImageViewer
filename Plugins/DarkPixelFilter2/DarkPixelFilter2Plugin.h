#ifndef DARKPIXELFILTER2PLUGIN_H
#define DARKPIXELFILTER2PLUGIN_H

// Qt
#include <QObject>

// Project
#include "../PluginExport.h"
#include "Filters/AbstractFilter.h"

namespace Plugins
{

//******************************************************************************

class GIV_PLUGIN_EXPORT DarkPixelFilter2Plugin : public Filters::AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "AbstractFilter")

    Q_PROPERTY_WITH_ACCESSORS(int, minSize, getMinSize, setMinSize)
    Q_CLASSINFO("minSize","label:Min object size (in pixels);minValue:1;maxValue:500")

    Q_PROPERTY_WITH_ACCESSORS(double, sensivity, getSensivity, setSensivity)
    Q_CLASSINFO("sensivity","label: Detection sensivity between 0 and 1;minValue:0;maxValue:1.0")

//    Q_PROPERTY_WITH_ACCESSORS(bool, linkAligned, linkAligned, setLinkAligned)
//    Q_CLASSINFO("linkAligned","label:Join aligned objects")


    PROPERTY_ACCESSORS(int, gbWinSize, getGBWinSize, setGBWinSize)
    PROPERTY_ACCESSORS(float, resizeFactor, getResizeFactor, setResizeFactor)
    PROPERTY_ACCESSORS(int, atWinSize, getATWinSize, setATWinSize)
    PROPERTY_ACCESSORS(int, mcWinSize, getMCWinSize, setMCWinSize)


public:
    DarkPixelFilter2Plugin(QObject* parent = 0);

protected:
    virtual cv::Mat filter(const cv::Mat &src) const;

};

//******************************************************************************

}

#endif // DARKPIXELFILTER2PLUGIN_H
