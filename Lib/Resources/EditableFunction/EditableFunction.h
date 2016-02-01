//******************************************************************************
// DLL Export definitions
//******************************************************************************
#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined EF_EXPORT_DEF
#  define EF_EXPORT __declspec(dllexport)
#elif (defined WIN32 || defined _WIN32 || defined WINCE)
#  define EF_EXPORT __declspec(dllimport) // This helps to resolve the problem with plugins link
#else
#  define EF_EXPORT
#endif

// ALL INCLUDES
#include <iostream>

// Opencv
#include <opencv2/core/core.hpp>

//******************************************************************************
// Local variables & functions
//******************************************************************************

cv::Mat filter(const cv::Mat & inputImg);

static float _noDataValue;

//******************************************************************************
// Exported functions
//******************************************************************************
extern "C" double EF_EXPORT foo(double value);

extern "C" bool EF_EXPORT filterFunc(uchar * inputData, int inputWidth, int inputHeight, int inputCvType,
                                     float noDataValue,
                                        uchar ** outputData, int * outputWidth, int * outputHeight, int * outputCvType)
{

    _noDataValue = noDataValue;

    if (!inputData ||
            !outputData ||
            !outputWidth ||
            !outputHeight ||
            !outputCvType)
    {
        std::cerr << "Some of attribute pointers is null" << std::endl;
        return false;
    }

    cv::Mat inputImg(inputHeight, inputWidth, inputCvType, (void*) inputData);

    cv::Mat res = filter(inputImg);

    *outputHeight = res.rows;
    *outputWidth = res.cols;
    *outputCvType = res.type();
    *outputData = res.data;

    // add ref to Matrix such that memory is not freed
    res.addref();
    return true;
}


