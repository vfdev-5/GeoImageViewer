
// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "DarkPixelFilterPlugin.h"
#include "Core/LayerUtils.h"


namespace Plugins
{

//******************************************************************************

/*!
  \class DarkPixelFilterPlugin
  \brief Plugin to segment dark pixel zones

  Method :
  1) Image -> 1.0/Image
  2) Convert to 8U
  3) Adaptive thresholding
  4) Morpho close+dilate
  5) Find contours
  6) Select contours by area: s > minSize^2

*/

//******************************************************************************

DarkPixelFilterPlugin::DarkPixelFilterPlugin(QObject *parent) :
    Filters::AbstractFilter(parent),
    _minSize(5)
{
    _name=tr("Dark Pixels Segmentation");
    _description=tr("Dark pixel zones segmentation");
}

//******************************************************************************

cv::Mat DarkPixelFilterPlugin::filter(const cv::Mat &data) const
{
    cv::Mat out;
    if (data.channels() > 1)
    {
        // create gray image
        std::vector<cv::Mat> iChannels(data.channels());
        cv::split(data, &iChannels[0]);

        double a = 1.0/iChannels.size();
        iChannels[0].convertTo(out, CV_32F, a);
        for (int i=1; i<iChannels.size();i++)
        {
            out += a*iChannels[i];
        }
    }
    else
    {
        if (data.depth() == CV_32F)
        {
            data.copyTo(out);
        }
        else
        {
            data.convertTo(out, CV_32F);
        }
    }

    // 1) Image -> 1/Image
    cv::divide(1.0,out,out,CV_32F);

    // 2) Convert to 8U
    double minVal, maxVal;
    cv::minMaxLoc(out, &minVal, &maxVal);
    cv::Mat out8U;
    out.convertTo(out8U, CV_8U, 255.0/(maxVal-minVal), -255.0*minVal/(maxVal-minVal));
    out = out8U;


    // 3) Adaptive thresholding
    int winsize = 131;
    cv::Scalar mean = cv::mean(out);
    // !!! FIND ANOTHER CRITERIUM
    double c = -mean[0];
    cv::adaptiveThreshold(out, out, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, winsize, c);

    // 4) Morpho close + open
    cv::Mat k1=cv::Mat::ones(5,5,CV_8U);
    cv::Mat k2=cv::Mat::ones(3,3,CV_8U);
    cv::morphologyEx(out, out, cv::MORPH_CLOSE, k1);
    cv::morphologyEx(out, out, cv::MORPH_OPEN, k2);

    // 5) Find contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(out, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    cv::Mat res(out.size(), CV_32F);
//    res.setTo(0.0);
    res.setTo(_noDataValue);

    if (contours.size() > 0)
    {
//        SD_TRACE("contours.size() > 0");
        double minArea = _minSize*_minSize;
        for (int i=0;i<contours.size();i++)
        {
            std::vector<cv::Point> contour = contours[i];
            if (contour.size() < 4)
                continue;
            double s = cv::contourArea(contour);
//            SD_TRACE(QString("area=%1").arg(s));
            if (s > minArea)
            {
//                SD_TRACE("s>minArea");
                cv::drawContours(res, contours, i, cv::Scalar::all(1.0), CV_FILLED);
//                SD_TRACE("cv::drawContours");
            }
        }
        out = res;
//        SD_TRACE("out=res");
    }
    else
    {
        _errorMessage = tr("No dark pixel zones found of size larger than %1").arg(_minSize);
        return cv::Mat();
    }

    return out;
}

//******************************************************************************

}
