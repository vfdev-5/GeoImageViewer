
// Opencv
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Do not remove this include
#include "EditableFunction.h"

cv::Mat filter(const cv::Mat & inputImg)
{
    std::cout << "DEBUG : Inside filter function" << std::endl;
    cv::Mat out;

    cv::blur(inputImg, out, cv::Size(3,3));



    return out;
}
