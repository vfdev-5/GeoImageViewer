
// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "BlurFilter.h"


namespace Filters
{

//******************************************************************************
/*!
  \class BlurFilter
  \brief Blur filter implementation

  */

//******************************************************************************

BlurFilter::BlurFilter(QObject *parent) :
    AbstractFilter(parent),
    _sizeX(3),
    _sizeY(3)
{
    _name = tr("Blur filter");
    _description = tr("Simple blur filter");
}

//******************************************************************************

cv::Mat BlurFilter::filter(const cv::Mat &src) const
{
    cv::Mat out;

    SD_TRACE(QString("Blur filter : size = %1, %2").arg(_sizeX).arg(_sizeY));

    cv::blur(src, out, cv::Size(_sizeX, _sizeY));

    return out;

}

//******************************************************************************

}
