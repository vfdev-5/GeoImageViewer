
// Qt
#include <qmath.h>

// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "DifferentialFilter.h"


namespace Filters
{

//******************************************************************************

/*!
  \class DifferentialFilter
  \brief Differential filter implementation : sobel, scharr, laplace
*/

//******************************************************************************

DifferentialFilter::DifferentialFilter(QObject *parent) :
    AbstractFilter(parent),
    _size(3),
    _type("sobel")
{
    _name = tr("Differential filter");
    _description = tr("Differential filter with options : sobel, scharr, laplacian\n Size parameters is not applied to Scharr type");
}

//******************************************************************************

cv::Mat DifferentialFilter::filter(const cv::Mat &src) const
{
    cv::Mat out;

    SD_TRACE1("Differential filter : type=%1", _type);
    if (_type=="sobel")
    {
        cv::Mat t1, t2;
        cv::Sobel(src, t1, src.depth(), 1, 0, _size);
        cv::Sobel(src, t2, src.depth(), 0, 1, _size);
        cv::magnitude(t1, t2, out);
    }
    else if (_type=="scharr")
    {
        cv::Mat t1, t2;
        cv::Scharr(src, t1, src.depth(), 1, 0);
        cv::Scharr(src, t2, src.depth(), 0, 1);
        cv::magnitude(t1, t2, out);
    }
    else if (_type=="laplacian")
    {
        cv::Laplacian(src, out, src.depth(), _size);
    }
    return out;
}

//******************************************************************************

}

