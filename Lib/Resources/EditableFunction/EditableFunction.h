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

// Std
#include <iostream>
#include <queue>

// Opencv
#include <opencv2/core/core.hpp>

//******************************************************************************
// Local variables & functions
//******************************************************************************

cv::Mat filter(const cv::Mat & inputImg);

static float _noDataValue;


typedef std::pair<std::string, cv::Mat> Message;
static std::queue<Message> _queue;
static cv::Mat _result;

void verboseDisplayImage(const std::string & name, const cv::Mat & img)
{
    cv::Mat copy = img.clone();
    _queue.push(Message(name, copy));
}

//******************************************************************************
// Exported functions
//******************************************************************************

extern "C" double EF_EXPORT foo(double value);

//******************************************************************************

extern "C" bool EF_EXPORT filterFuncP1(uchar * inputData, int inputWidth, int inputHeight, int inputCvType,
                                       float noDataValue,
                                       int * outputWidth, int * outputHeight, int * outputCvType)
{
    _queue.swap( std::queue<Message>() );
    _noDataValue = noDataValue;

    if (!inputData ||
            !outputWidth ||
            !outputHeight ||
            !outputCvType)
    {
        std::cerr << "Some of attribute pointers is null" << std::endl;
        return false;
    }

    cv::Mat inputImg(inputHeight, inputWidth, inputCvType, (void*) inputData);

    try
    {
        _result = filter(inputImg);
    }
    catch (const cv::Exception & e)
    {
        std::cerr << "OpenCV Exception : " << e.msg << std::endl;
        return false;
    }


    if (!_result.empty())
    {
        *outputHeight = _result.rows;
        *outputWidth = _result.cols;
        *outputCvType = _result.type();
    }
    else
    {
        *outputHeight = 0;
        *outputWidth = 0;
        *outputCvType = -1;
    }
    return true;
}

//******************************************************************************

extern "C" bool EF_EXPORT filterFuncP2(uchar * outputData)
{
    if (!outputData)
    {
        std::cerr << "Output data pointer is null" << std::endl;
        return false;
    }
    // copy data:
    memcpy(outputData, _result.data, _result.rows * _result.cols * _result.elemSize());
    _result.release();
    return true;
}

//******************************************************************************

extern "C" int EF_EXPORT verboseStackCount()
{
    return _queue.size();
}

//******************************************************************************

extern "C" bool EF_EXPORT verboseStackNextP1(int * outputWidth, int *outputHeight, int *outputCvType, int * msgsize)
{
    if (!outputWidth ||
            !outputHeight ||
            !outputCvType ||
            !msgsize)
    {
        std::cerr << "Some of attribute pointers is null" << std::endl;
        return false;
    }

    Message & msg = _queue.front();
    *msgsize = msg.first.size();
    cv::Mat res = msg.second;
    if (!res.empty())
    {
        *outputHeight = res.rows;
        *outputWidth = res.cols;
        *outputCvType = res.type();
    }
    else
    {
        *outputHeight =  0;
        *outputWidth = 0;
        *outputCvType = -1;
    }
    return true;

}

//******************************************************************************

extern "C" bool EF_EXPORT verboseStackNextP2(uchar * outputData, char * msgdata)
{
    if (!msgdata || !outputData)
    {
        std::cerr << "Some of attribute pointers is null" << std::endl;
        return false;
    }

    Message & msg = _queue.front();
    strcpy(msgdata, msg.first.c_str());

    cv::Mat res = msg.second;
    memcpy(outputData, res.data, res.rows * res.cols * res.elemSize());
    _queue.pop();
    return true;
}

//******************************************************************************
