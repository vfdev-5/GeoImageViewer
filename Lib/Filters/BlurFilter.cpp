
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

//    cv::Mat noDataMask = src != _noDataValue;
//    int nbBands = src.channels();

//    if (nbBands != noDataMask.channels())
//    {
//        SD_TRACE("BlurFilter::filter : Source image nb of channels != noDataMask nb of channels");
//        return out;
//    }

//    std::vector<cv::Mat> iChannels(nbBands);
//    std::vector<cv::Mat> oChannels(nbBands);
//    std::vector<cv::Mat> mChannels(nbBands);
//    cv::split(src, &iChannels[0]);
//    cv::split(noDataMask, &mChannels[0]);

//    for (int i=0;i<nbBands;i++)
//    {
//        // set to zero all pixels with noDataValues
//        iChannels[i] *= mChannels[i];

//    }

    cv::blur(src, out, cv::Size(_sizeX, _sizeY));

    return out;

}

//******************************************************************************

}

