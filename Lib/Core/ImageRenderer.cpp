
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
/*!
 * \brief ImageRenderer::setToRGBMapping method to setup default 'Bands-to-RGB' mapping
 * \param layer
 * \return
 */

bool ImageRenderer::setToRGBMapping(ImageDataProvider *provider)
{
    int nbBands=provider->getNbBands();
    bool isComplex=provider->isComplex();
    _conf.toRGBMapping.clear();
    if (nbBands == 1 && !isComplex)
    {
        _conf.toRGBMapping.insert(0, 0);
        _conf.toRGBMapping.insert(1, 0);
        _conf.toRGBMapping.insert(2, 0);
    }
    else if (nbBands >= 1 && isComplex)
    {
        // TODO !!! NEED TO CREATE VIRTUAL BANDS
        SD_ERR(QObject::tr("Complex images are not yet supported"));
    }
    else if (provider->getNbBands() > 2 && !isComplex)
    { // Image with more the 2 not complex bands is interpreted as RGB image
        _conf.toRGBMapping.insert(0, 0);
        _conf.toRGBMapping.insert(1, 1);
        _conf.toRGBMapping.insert(2, 2);
    }
    else if (provider->getNbBands() == 2 && !isComplex)
    { // Image with 2 bands is not interpreted as complex image and displayed as gray level image
        _conf.toRGBMapping.insert(0, 0);
        _conf.toRGBMapping.insert(1, 0);
        _conf.toRGBMapping.insert(2, 0);
    }
    else
    {
        SD_TRACE("ImageRenderer::setToRGBMapping : Failed to create toRGB mapping");
        return false;
    }
    return true;
}

//******************************************************************************

bool ImageRenderer::setupConfiguration(ImageDataProvider *dataProvider)
{

    _conf.minValues = dataProvider->getMinValues();
    _conf.maxValues = dataProvider->getMaxValues();

    if (!setToRGBMapping(dataProvider))
    {
        return false;
    }

    return true;
}

//******************************************************************************

void ImageRenderer::setConfiguration(const ImageRendererConfiguration & conf)
{
    _conf = conf;
}

//******************************************************************************

inline bool ImageRenderer::checkBeforeRender(const cv::Mat & rawData)
{
    int nbBands = rawData.channels();
    if (nbBands != _conf.minValues.size() ||
            nbBands != _conf.maxValues.size())
        return false;
    if (_conf.toRGBMapping.size() != 3)
        return false;

    return true;
}

//******************************************************************************
/*!
 * \brief ImageRenderer::render transforms raw data using min/max values into RGBA (32 bits) format
 * \param rawData
 * \return Matrix in RGBA 32-bits format, 4 channels
 */
cv::Mat ImageRenderer::render(const cv::Mat &oRawData, bool isBGRA)
{
    cv::Mat outputImage8U;
    if (!checkBeforeRender(oRawData))
        return outputImage8U;

    // Get alpha channel and rewrite noDataValues to zero
    cv::Mat mask, alpha8U = oRawData > ImageDataProvider::NoDataValue;
    alpha8U.convertTo(mask, oRawData.depth());
    cv::Mat rawData = oRawData.mul(mask);

    int nbBands = rawData.channels();
    const QVector<int> & mapping = _conf.toRGBMapping;

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

       a = 255.0 / ( _conf.maxValues[index] - _conf.minValues[index] );
       b = - 255.0 * _conf.minValues[index] / ( _conf.maxValues[index] - _conf.minValues[index] );
       iChannels[index].convertTo(oChannels[i], CV_8U, a, b);
    }

    cv::merge(oChannels, outputImage8U);
    if (isBGRA)
        cv::cvtColor(outputImage8U, outputImage8U, CV_RGBA2BGRA);
    return outputImage8U;
}

//******************************************************************************

}
