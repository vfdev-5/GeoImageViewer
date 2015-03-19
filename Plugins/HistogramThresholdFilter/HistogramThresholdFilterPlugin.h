#ifndef HistogramThresholdFilterPlugin_H
#define HistogramThresholdFilterPlugin_H

// Qt
#include <QObject>
#include <QEvent>

// Project
#include "HistogramThreshold.h"
#include "Filters/AbstractFilter.h"

namespace Plugins
{

//******************************************************************************

class GIV_PLUGIN_EXPORT HistogramThresholdFilterPlugin : public Filters::AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "AbstractFilter")

public:

    HistogramThresholdFilterPlugin(QObject * parent = 0) :
        Filters::AbstractFilter(parent)
    {
        _name=tr("Histogram Threshold");
        _description=tr("Filter to segment image using histogram thresholding");
    }

protected:

    virtual cv::Mat filter(const cv::Mat & ) const;

};

//******************************************************************************

}

#endif // HistogramThresholdFilterPlugin_H
