
// OpenCV
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "LayerUtils.h"
#include "ImageRenderer.h"
#include "ImageDataProvider.h"

namespace Core
{

//******************************************************************************

ImageRenderer::ImageRenderer(QObject *parent) :
    QObject(parent)
{
}

//******************************************************************************

bool ImageRenderer::setupConfiguration(const ImageDataProvider *dataProvider, ImageRendererConfiguration * conf)
{
    if (!dataProvider || !conf)
        return false;
    conf->minValues = dataProvider->getMinValues();
    conf->maxValues = dataProvider->getMaxValues();
    conf->toRGBMapping = computeToRGBMapping(dataProvider);
    return !conf->toRGBMapping.isEmpty();
}

//******************************************************************************

//void ImageRenderer::setConfiguration(const ImageRendererConfiguration & conf)
//{
//    _conf = conf;
//}

//******************************************************************************
/*!
 * \brief ImageRenderer::render transforms raw data using min/max values into RGBA (32 bits) format
 * \param rawData
 * \return Matrix in RGBA 32-bits format, 4 channels
 */
cv::Mat ImageRenderer::render(const cv::Mat &oRawData, const ImageRendererConfiguration * conf, bool isBGRA)
{
    cv::Mat outputImage8U;
    if (!checkBeforeRender(oRawData.channels(), conf))
        return outputImage8U;

    // Get alpha channel and rewrite noDataValues to zero
    cv::Mat mask, alpha8U = oRawData > ImageDataProvider::NoDataValue;
    alpha8U.convertTo(mask, oRawData.depth());
    cv::Mat rawData = oRawData.mul(mask);

    int nbBands = rawData.channels();
    const QVector<int> & mapping = conf->toRGBMapping;

    std::vector<cv::Mat> iAlpha8U(nbBands);
    std::vector<cv::Mat> iChannels(nbBands);
    // Include alpha channel as the last band <-> RGBA
    std::vector<cv::Mat> oChannels(mapping.size() + 1);
    cv::split(rawData, &iChannels[0]);
    cv::split(alpha8U, &iAlpha8U[0]);

    // render:
    // set alpha channel:
    oChannels[3] = iAlpha8U[0];
    for (int i=0; i < mapping.size(); i ++)
    {
       int index = mapping[i];
       double a(1.0);
       double b(0.0);

       a = 255.0 / ( conf->maxValues[index] - conf->minValues[index] );
       b = - 255.0 * conf->minValues[index] / ( conf->maxValues[index] - conf->minValues[index] );
       iChannels[index].convertTo(oChannels[i], CV_8U, a, b);
    }

    cv::merge(oChannels, outputImage8U);
    if (isBGRA)
        cv::cvtColor(outputImage8U, outputImage8U, CV_RGBA2BGRA);
    return outputImage8U;
}

//******************************************************************************
//******************************************************************************
/*!
 * \brief computeToRGBMapping method to compute default 'Bands-to-RGB' mapping
 * \param ImageDataProvider
 * \return toRGBMapping as QVector<int>
 */

QVector<int> computeToRGBMapping(const ImageDataProvider *provider)
{
    QVector<int> toRGBMapping;
    bool isComplex=provider->inputIsComplex();
    int nbBands=provider->getInputNbBands();
    if (!isComplex)
    {
        if (nbBands == 1)
        {
            toRGBMapping.insert(0, 0);
            toRGBMapping.insert(1, 0);
            toRGBMapping.insert(2, 0);
        }
        else if (provider->getNbBands() > 2 )
        { // Image with more the 2 not complex bands is interpreted as RGB image
            toRGBMapping.insert(0, 0);
            toRGBMapping.insert(1, 1);
            toRGBMapping.insert(2, 2);
        }
        else if (provider->getNbBands() == 2)
        { // Image with 2 bands is not interpreted as complex image and displayed as gray level image
            toRGBMapping.insert(0, 0);
            toRGBMapping.insert(1, 0);
            toRGBMapping.insert(2, 0);
        }
        else
        {
            SD_TRACE("ImageRenderer::setToRGBMapping : Failed to create toRGB mapping");
        }
    }
    else
    {
        if (nbBands >= 1)
        {
            // Single or Multi-bands Complex image -> choose Abs channel of the 1st band
            toRGBMapping.insert(0, 2);
            toRGBMapping.insert(1, 2);
            toRGBMapping.insert(2, 2);
        }
        else
        {
            SD_TRACE("ImageRenderer::setToRGBMapping : Failed to create toRGB mapping");
        }
    }
    return toRGBMapping;
}

//******************************************************************************

}
