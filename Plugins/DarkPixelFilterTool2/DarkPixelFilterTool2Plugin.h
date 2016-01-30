#ifndef DarkPixelFilterTool2Plugin_H
#define DarkPixelFilterTool2Plugin_H

// Qt
#include <QObject>

// Opencv
#include <opencv2/core/core.hpp>

// Project
#include "../PluginExport.h"
#include "Tools/FilterTool.h"

class QGraphicsItem;

namespace Plugins
{

//******************************************************************************

class GIV_PLUGIN_EXPORT DarkPixelFilterTool2Plugin : public Tools::FilterTool
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "FilterTool")

    Q_PROPERTY_WITH_ACCESSORS(double, param, getParam, setParam)
    Q_CLASSINFO("param","label:More detections;minValue:-1000.0;maxValue:1000.0")

    Q_PROPERTY_WITH_ACCESSORS(int, minSize, getMinSize, setMinSize)
    Q_CLASSINFO("minSize","label:Min object size (in pixels);minValue:1;maxValue:500")

public:
    DarkPixelFilterTool2Plugin(QObject * parent = 0);

protected slots:
    virtual void onFinalize();

protected:
    virtual cv::Mat processData(const cv::Mat & data);

};

//******************************************************************************

}

#endif // DarkPixelFilterToolPlugin2
