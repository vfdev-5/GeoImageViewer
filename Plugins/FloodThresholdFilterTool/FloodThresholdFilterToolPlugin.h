#ifndef FloodThresholdFilterToolPlugin_H
#define FloodThresholdFilterToolPlugin_H

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

class GIV_PLUGIN_EXPORT FloodThresholdFilterToolPlugin : public Tools::FilterTool
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "FilterTool")

//    Q_PROPERTY_WITH_ACCESSORS(int, threshold, getThreshold, setThreshold)
//    Q_CLASSINFO("threshold","minValue:0;maxValue:10000")

    Q_PROPERTY_WITH_ACCESSORS(int, loDiff, getLoDiff, setLoDiff)
    Q_CLASSINFO("loDiff","label:Lower difference;minValue:0;maxValue:10000")

    Q_PROPERTY_WITH_ACCESSORS(int, upDiff, getUpDiff, setUpDiff)
    Q_CLASSINFO("upDiff","label:Upper difference;minValue:0;maxValue:10000")

//    Q_PROPERTY_WITH_ACCESSORS(bool, inverse, inverse, setInverse)



public:

    FloodThresholdFilterToolPlugin(QObject * parent = 0);

protected slots:
    virtual void onFinalize();

protected:
    virtual cv::Mat processData(const cv::Mat & data);

};

//******************************************************************************

}

#endif // FloodThresholdFilterToolPlugin_H
