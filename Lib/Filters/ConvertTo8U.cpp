
// Qt
#include <qmath.h>

// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "ConvertTo8U.h"
#include "Core/LayerUtils.h"


namespace Filters
{

//******************************************************************************

/*!
  \class ConvertTo8U
  \brief Filter to convert single channel image to 8 bits using methods :
        1) out = 255 * (in - min(in))/(max(in) - min(in))
        2) out = 255 * (in - umin)/(umax - umin)
        3) out = 255 * (in - min2))/(max2 - min2) with min2 = mean(in) - a*std(in), max2 = mean(in) + a*std(in)
        4) out = 255 * (in - min3))/(max3 - min3) with min3 = quantile(in, a), max3 = quantile(in, b),
*/

//******************************************************************************

ConvertTo8U::ConvertTo8U(QObject *parent) :
    AbstractFilter(parent),
    _type("real min/max"),
    _stdFactor(3.0),
    _minQuantile(5.0),
    _maxQuantile(95.0),
    _umin(0.0),
    _umax(1e6)
{
    _name = tr("Convert to 8U");
    _description = tr("Filter to convert single channel image to 8 bits using different methods.\n 'real min/max' : out = 255 * (in - min(in))/(max(in) - min(in)) \n 'user min/max' : out = 255 * (in - umin)/(umax - umin) \n 'use mean/std' : out = 255 * (in - min2))/(max2 - min2) with min2 = mean(in) - a*std(in), max2 = mean(in) + a*std(in) \n 'quantiles' : out = 255 * (in - min3))/(max3 - min3) with min3 = quantile(in, a), max3 = quantile(in, b),");
}

//******************************************************************************

cv::Mat ConvertTo8U::filter(const cv::Mat &src) const
{
    cv::Mat out;

    if (src.channels() > 1)
        return out;

    if (src.depth() == CV_8U)
        return src;

    SD_TRACE1("ConvertTo8U : type=%1", _type);
    double minValue(0.0), maxValue(1.0);
    if (_type=="real min/max")
    {
        cv::minMaxLoc(src, &minValue, &maxValue);
    }
    else if (_type=="use mean/std")
    {
        cv::Scalar mean, std;
        cv::meanStdDev(src, mean, std);
        minValue = mean[0] - _stdFactor*std[0];
        minValue = mean[0] + _stdFactor*std[0];
    }
    else if (_type=="quantiles")
    {
        QVector<double> minValues, maxValues;
        QVector< QVector<double> > bandHistograms;
        if (Core::computeNormalizedHistogram(src, cv::Mat(), minValues, maxValues, bandHistograms))
        {
            Core::computeQuantileMinMaxValue(bandHistograms[0], _minQuantile, _maxQuantile, minValues[0], maxValues[0], &minValue, &maxValue);
        }
    }
    else if (_type=="user min/max")
    {
        minValue = _umin;
        maxValue = _umax;
    }
    SD_TRACE2("ConvertTo8U : min/max : %1, %2", minValue, maxValue);
    src.convertTo(out, CV_8U, 255.0/(maxValue - minValue), -255.0*minValue/(maxValue - minValue));

    return out;
}

//******************************************************************************

}

