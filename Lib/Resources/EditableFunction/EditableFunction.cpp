
// Opencv
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Do not remove this include
#include "EditableFunction.h"


// Built-in functions :
//
// verboseDisplayImage(const std::string & name, const cv::Mat & img)
//

cv::Mat filter(const cv::Mat & inputImg)
{
    std::cout << "DEBUG : Inside filter function" << std::endl;
    cv::Mat out;

    cv::blur(inputImg, out, cv::Size(3,3));

    verboseDisplayImage("Blur", out);

    cv::threshold(out, out, 122, 255, CV_THRESH_BINARY);

    verboseDisplayImage("Threshold", out);

    return out;
}
