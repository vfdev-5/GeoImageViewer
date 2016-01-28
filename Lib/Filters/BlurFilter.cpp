
// Qt
#include <qmath.h>

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
    _sizeY(3),
    _type("mean"),
    _sigmaX(1.0),
    _sigmaY(1.0)
{
    _name = tr("Blur filter");
    _description = tr("Blur filter with options : mean, median, gaussian");
}

//******************************************************************************

cv::Mat BlurFilter::filter(const cv::Mat &src) const
{
    cv::Mat out;

    SD_TRACE3("Blur filter : type=%1, size = %2, %3", _type, _sizeX, _sizeY);
    if (_type=="mean")
    {
        cv::blur(src, out, cv::Size(_sizeX, _sizeY));
    }
    else if (_type=="median")
    {
        cv::medianBlur(src,out,qMax(_sizeX, _sizeY));
    }
    else if (_type=="gaussian")
    {
        SD_TRACE2("Sigma X/Y : %1, %2", _sigmaX, _sigmaY);
        cv::GaussianBlur(src, out, cv::Size(_sizeX, _sizeY), _sigmaX, _sigmaY);
    }
    return out;
}

//******************************************************************************

}

