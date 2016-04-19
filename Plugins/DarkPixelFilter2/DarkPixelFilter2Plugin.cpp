// Qt
#include <qmath.h>

// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "DarkPixelFilter2Plugin.h"


namespace Plugins
{

cv::Mat blurThreshClose(const cv::Mat & data, int blurSize=3, int threshold=200, int closeSize=3, int n=1)
{
    cv::Mat out;
    cv::blur(data, out, cv::Size(blurSize, blurSize));

    cv::threshold(out, out, threshold, 255, cv::THRESH_BINARY);

    cv::Mat k1=cv::Mat::ones(closeSize, closeSize,CV_8U);
    cv::morphologyEx(out, out, cv::MORPH_CLOSE, k1, cv::Point(-1,-1), n);

    return out;
}

//******************************************************************************

/*!
  \class DarkPixelFilter2Plugin
  \brief Plugin to segment dark pixel zones

  Method :
  -) Image -> 1.0/Image
  -) Gaussian blur
  -) Resize to small
  -) Convert to 8U
  -) Median blur
  -) Only bright objects
  -) Adaptive thresholding
  -) blur+threshold+Morpho close
  -) Resize to normal
  -) Find contours
  -) Select contours by area: s > minSize^2
*/

//******************************************************************************

DarkPixelFilter2Plugin::DarkPixelFilter2Plugin(QObject *parent) :
    Filters::AbstractFilter(parent),
    _minSize(15),
    _sensivity(0.5),
    _gbWinSize(9),
    _resizeFactor(0.15),
    _atWinSize(131),
    _mcWinSize(3)
{
    _name=tr("Dark Pixels Segmentation 2");
    _description=tr("Dark pixel zones segmentation (algorithm 2)");
}

//******************************************************************************

cv::Mat DarkPixelFilter2Plugin::filter(const cv::Mat &data) const
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

    // -) Compute mask from specified value:
    cv::Mat datamask;
    if (_maskByValue > -12345)
    {
        datamask = out != _maskByValue;
    }


    // -) Image -> 1/Image
    cv::divide(1.0,out,out,CV_32F);

    if (_verbose) { verboseDisplayImage("Image -> 1/Image", out); }
    emit progressValue(15);


    // -) Gaussian blur
    cv::GaussianBlur(out, out, cv::Size(_gbWinSize, _gbWinSize), _gbWinSize/1.5);
    if (_verbose) { verboseDisplayImage("Gaussian blur", out); }


    // -) Resize image
    int initW=out.cols, initH=out.rows;
    double rf = _resizeFactor;
    int newW = initW*rf;
    int newH = initH*rf;
    if (newW < 256 || newH < 256)
    {
        rf = qMax(256.0 / initW, 256.0 / initH);
        SD_TRACE1("New resize factor : %1", rf);
    }

    cv::resize(out, out, cv::Size(0, 0), rf, rf, cv::INTER_NEAREST);
    if (!datamask.empty())
    {
        cv::resize(datamask, datamask, cv::Size(0, 0), rf, rf, cv::INTER_NEAREST);
    }

    emit progressValue(25);

    // -) Convert to 8U
    {
        double minVal, maxVal;
        cv::minMaxLoc(out, &minVal, &maxVal, 0, 0,datamask);
        cv::Scalar mean, std;
        cv::meanStdDev(out, mean, std, datamask);
        double nmin = mean.val[0] - 3.0*std.val[0];
        minVal = (nmin < minVal) ? minVal : nmin;
        double nmax = mean.val[0] + 3.0*std.val[0];
        maxVal = (nmax > maxVal) ? maxVal : nmax;
        cv::Mat out8U;
        out.convertTo(out8U, CV_8U, 255.0/(maxVal-minVal), -255.0*minVal/(maxVal-minVal));
        out = out8U;
        emit progressValue(30);
    }

    // -) Median blur
    cv::medianBlur(out, out, 3);

    // -) Only bright objects
    {
        cv::Scalar mean = cv::mean(out, datamask);
        cv::threshold(out, out, mean[0], 255, cv::THRESH_TOZERO);
        cv::Mat mask, m = out == 0;
        m.convertTo(mask, out.type(), mean[0]/255.0);
        out = out + mask;
    }
    if (_verbose) { verboseDisplayImage("Only bright objects", out); }

    // -) Adaptive thresholding

    // Y = (Ymax - Ymin) * (X - Xmin)/(Xmax - Xmin) + Ymin
    // Y = (Ymin - Ymax) * (X - Xmin)/(Xmax - Xmin) + Ymax
    // transform sensivity [0.0 -> 1.0] into coeff [1.0 -> 0.3]
    double v = 1.0 - (0.7)*_sensivity;
    cv::Scalar mean = cv::mean(out, datamask);
    double c = -v*mean[0];
    cv::adaptiveThreshold(out, out, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, _atWinSize, c);

    if (_verbose) { verboseDisplayImage("Adaptive thresholding", out); }
    emit progressValue(40);

    // -) Morpho close
    out = blurThreshClose(out, 3, 100, _mcWinSize, 2);
    if (_verbose) { verboseDisplayImage("Blur + Theshold + Morpho", out); }
    emit progressValue(50);

    // -) Resize to initial
    cv::resize(out, out, cv::Size(initW, initH), 0, 0, cv::INTER_LINEAR);
    emit progressValue(60);

    // -) Make contours smoother
    {
        int s = 7;
        cv::blur(out, out, cv::Size(s, s));
        cv::Mat k1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(s, s));
        cv::morphologyEx(out, out, cv::MORPH_ERODE, k1);
    }

    // -) Find contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(out, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    cv::Mat res(out.size(), CV_32F);
    res.setTo(_noDataValue);
    emit progressValue(80);

    if (contours.size() > 0)
    {
        double minArea = _minSize*_minSize;
        for (int i=0;i<contours.size();i++)
        {
            std::vector<cv::Point> contour = contours[i];
            // remove line and points
            if (contour.size() < 3)
                continue;
            double s = cv::contourArea(contour);
            if (s > minArea)
            {
                cv::drawContours(res, contours, i, cv::Scalar::all(1.0), CV_FILLED);
            }
        }
        out = res;
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
