


// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "PowerFilter.h"


namespace Filters
{

//******************************************************************************
/*!
  \class PowerFilter
  \brief applies power operation on input image
*/
//******************************************************************************

PowerFilter::PowerFilter(QObject *parent) :
    AbstractFilter(parent),
    _power(2.0)
{
    _name = tr("Power filter");
    _description = tr("Apply power to image pixels");
}

//******************************************************************************

cv::Mat PowerFilter::filter(const cv::Mat &src) const
{
    cv::Mat out;
    SD_TRACE(QString("Power filter : power = %1").arg(_power));
    cv::pow(src, _power, out);
    return out;
}

//******************************************************************************

}

