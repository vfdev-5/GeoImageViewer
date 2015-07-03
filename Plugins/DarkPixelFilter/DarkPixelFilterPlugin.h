#ifndef DARKPIXELFILTERPLUGIN_H
#define DARKPIXELFILTERPLUGIN_H

// Qt
#include <QObject>

// Project
#include "../PluginExport.h"
#include "Filters/AbstractFilter.h"

namespace Plugins
{

//******************************************************************************

class GIV_PLUGIN_EXPORT DarkPixelFilterPlugin : public Filters::AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "AbstractFilter")

    Q_PROPERTY_WITH_ACCESSORS(int, minSize, getMinSize, setMinSize)
    Q_CLASSINFO("minSize","label:Min object size (in pixels);minValue:1;maxValue:500")


public:
    DarkPixelFilterPlugin(QObject* parent = 0);

protected:
    virtual cv::Mat filter(const cv::Mat &src) const;

};

//******************************************************************************

}

#endif // DARKPIXELFILTERPLUGIN_H
