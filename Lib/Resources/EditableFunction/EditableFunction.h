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
// Local functions
//******************************************************************************

cv::Mat filter(const cv::Mat & inputImg);

typedef std::pair<std::string,cv::Mat> Message;
//static std::vector<Message > _stack;
static std::queue<Message > _queue;

void verboseDisplayImage(const std::string & name, const cv::Mat & img)
{
    _queue.push(Message(name, img));
}

//******************************************************************************
// Exported functions
//******************************************************************************
extern "C" double EF_EXPORT foo(double value);

extern "C" bool EF_EXPORT filterFunc(uchar * inputData, int inputWidth, int inputHeight, int inputCvType,
                                     uchar ** outputData, int * outputWidth, int * outputHeight, int * outputCvType)
{
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

extern "C" int EF_EXPORT verboseStackCount()
{
    return _queue.size();
}


extern "C" bool EF_EXPORT verboseStackNext(uchar ** outputData, int * outputWidth, int *outputHeight, int *outputCvType,
                                           char ** cstring)
{
    if (!cstring ||
            !outputData ||
            !outputWidth ||
            !outputHeight ||
            !outputCvType)
    {
        std::cerr << "Some of attribute pointers is null" << std::endl;
        return false;
    }

    std::cout << "DEBUG : Take first message" << std::endl;


    Message & msg = _queue.front();

    std::cout << "DEBUG : Allocate memory to copy a string" << std::endl;
    *cstring = new char(msg.first.size());
    std::cout << "DEBUG : Copy string" << std::endl;
    strcpy(*cstring, msg.first.c_str());

    std::cout << "DEBUG : Get matrix" << std::endl;
    cv::Mat res = msg.second;
    *outputHeight = res.rows;
    *outputWidth = res.cols;
    *outputCvType = res.type();
    *outputData = res.data;

    std::cout << "DEBUG : addref" << std::endl;

    // add ref to Matrix such that memory is not freed
    res.addref();

    _queue.pop();

    return true;

}

