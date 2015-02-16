
// Qt

// Project
#include "LayerRenderer.h"
#include "ImageLayer.h"

namespace Core
{

//******************************************************************************

LayerRenderer::LayerRenderer()
{
}

//******************************************************************************
/*!
 * \brief LayerRenderer::setToRGBMapping method to setup default 'Bands-to-RGB' mapping
 * \param layer
 * \return
 */

bool LayerRenderer::setToRGBMapping(ImageLayer *layer)
{
    int nbBands=layer->getNbBands();
    bool isComplex=layer->isComplex();
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
    else if (layer->getNbBands() > 2 && !isComplex)
    { // Image with more the 2 not complex bands is interpreted as RGB image
        _conf.toRGBMapping.insert(0, 0);
        _conf.toRGBMapping.insert(1, 1);
        _conf.toRGBMapping.insert(2, 2);
    }
    else if (layer->getNbBands() == 2 && !isComplex)
    { // Image with 2 bands is not interpreted as complex image and displayed as gray level image
        _conf.toRGBMapping.insert(0, 0);
        _conf.toRGBMapping.insert(1, 0);
        _conf.toRGBMapping.insert(2, 0);
    }
    else
    {
        SD_TRACE("LayerRenderer::setToRGBMapping : Failed to create toRGB mapping");
        return false;
    }
    return true;
}

//******************************************************************************

bool LayerRenderer::setupConfiguration(ImageLayer *layer)
{

    _conf.minValues = layer->getMinValues();
    _conf.maxValues = layer->getMaxValues();

    if (!setToRGBMapping(layer))
    {
        return false;
    }

    return true;
}

//******************************************************************************

void LayerRenderer::setConfiguration(const LayerRendererConfiguration & conf)
{
    _conf = conf;
}

//******************************************************************************

bool LayerRenderer::checkBeforeRender(const cv::Mat & rawData)
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

cv::Mat LayerRenderer::render(const cv::Mat &rawData)
{
    cv::Mat outputImage8U;
    if (!checkBeforeRender(rawData))
        return outputImage8U;

    int nbBands = rawData.channels();
    const QVector<int> & mapping = _conf.toRGBMapping;

    std::vector<cv::Mat> iChannels(nbBands);
    std::vector<cv::Mat> oChannels(mapping.size());
    cv::split(rawData, &iChannels[0]);

    // render:
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
    return outputImage8U;
}

//******************************************************************************

}
