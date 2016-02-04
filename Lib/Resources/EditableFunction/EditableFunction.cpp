// Std
#include <algorithm>
#include <vector>

// Opencv
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Do not remove this include
#include "EditableFunction.h"

// Predefined variables :
// float _noDataValue

// Built-in functions :
//
// verboseDisplayImage(const std::string & name, const cv::Mat & img)
//

int _minSize = 5;
int _loDiff = 2;
int _upDiff = 1;


cv::Mat blurThreshClose(const cv::Mat & data, int blurSize=3, int threshold=200, int closeSize=3, int n=1)
{
    cv::Mat out;
    cv::blur(data, out, cv::Size(blurSize, blurSize));

    cv::threshold(out, out, threshold, 255, cv::THRESH_BINARY);

    cv::Mat k1=cv::Mat::ones(closeSize, closeSize,CV_8U);
    cv::morphologyEx(out, out, cv::MORPH_CLOSE, k1, cv::Point(-1,-1), n);

    return out;
}

/*
    Compute spatial density
*/
float spatDensity(const cv::Mat & binImg)
{
    int n = 8;
    // tile image at n*n tiles and compute median density
    int tw = std::ceil(binImg.cols * 1.0 / n);
    int th = std::ceil(binImg.rows * 1.0 / n);

    int n2 = n*n;
    std::vector<float> d(n2, 0.0);

    cv::Rect r(0,0,0,0);
    int count=0;
    for (int i=0; i<n; i++)
    {
        r.x = tw*i;
        r.width = (r.x + tw > binImg.cols) ? binImg.cols - r.x : tw;
        for (int j=0; j<n; j++)
        {
            r.y = th*j;
            r.height = (r.y + th > binImg.rows) ? binImg.rows - r.y : th;
            cv::Mat tile = binImg(r);
//			std::stringstream ss;
//			ss << "tile_" << count;
//			verboseDisplayImage(ss.str(), tile);
            d[count] = cv::countNonZero(tile) * 1.0/ (tile.rows * tile.cols);
//            std::cout << "d[" << count << "]=" << d[count] << std::endl;
            count++;
        }
    }

    std::sort(d.begin(), d.end());
    return d[n2/2];
}


cv::Mat filter(const cv::Mat & data)
{
    std::cout << "DEBUG : Inside filter function" << std::endl;

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

//    verboseDisplayImage("Image -> 1/Image", out);

    cv::blur(out, out, cv::Size(3,3));

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
    verboseDisplayImage("Adaptive thresholding", out);


    float d = spatDensity(out);
    std::cout << "White Spatial Density : " << d << std::endl;

    //cv::Mat in = out.clone(); // input for floodFill

    // Median blur
    cv::medianBlur(out, out, 3);
    verboseDisplayImage("Image -> median(Image)", out);

    // 4) Morpho open
//    int size = _minSize/2 % 2 ? _minSize : _minSize - 1;
    int size = 3;

    cv::Mat k2 = cv::Mat::ones(size,size,CV_8U);
    cv::morphologyEx(out, out, cv::MORPH_CLOSE, k2);
//    verboseDisplayImage("Morpho close", out);

    cv::Mat k1=cv::Mat::ones(3, 3,CV_8U);
    cv::morphologyEx(out, out, cv::MORPH_OPEN, k1);
    verboseDisplayImage("Morpho open", out);


    // 5) Blur+Threshold
    int t=40, s=11, ss=7, n=5;
    out = blurThreshClose(out, s, t, ss, n);
    verboseDisplayImage("blur+thresh+close 1", out);
    out = blurThreshClose(out, s, t, ss, n);
    verboseDisplayImage("blur+thresh+close 2", out);

/*
    // 5) flood
    cv::Mat processedData(in.rows+2, in.cols+2, CV_8U, cv::Scalar(0));
    int flag = 4 | (255 << 8) | cv::FLOODFILL_FIXED_RANGE | cv::FLOODFILL_MASK_ONLY;
//    int i=490, j=259;
    for (int j=0; j<in.rows; j++)
    {
        for (int i=0; i<in.cols; i++)
        {
         if (out.at<uchar>(j,i) == 255)
         {
//            std::cout << "Run flood fill from seed point : " << i << ", " << j << std::endl;
//            std::cout << "In value at seed point and 4NN: " << (int) in.at<uchar>(j,i) << ", "
//					<< (int) in.at<uchar>(j,i+1) << ", "
//					<< (int) in.at<uchar>(j+1,i) << ", "
//					<< (int) in.at<uchar>(j+1,i+1) << std::endl;

                cv::Point p(i,j);
                cv::floodFill(in, processedData, p, cv::Scalar(255), 0, cv::Scalar::all(_loDiff), cv::Scalar::all(_upDiff), flag);

         }
        }
    }
    out = processedData(cv::Rect(1,1,processedData.cols-2,processedData.rows-2));

    verboseDisplayImage("Flood fill", out);
*/
    // 6) insert noDataValue
    cv::Mat m = out == 0;
    cv::Mat mask;
    m.convertTo(mask, CV_32F, 1.0/255.0);
    mask = mask.mul(_noDataValue);
    cv::Mat o;
    out.convertTo(o, CV_32F, 1.0/255.0);
    out = o + mask;


    return out;
}
