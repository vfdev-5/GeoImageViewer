
// Project
#include "HistogramLayerRenderer.h"
#include "ImageLayer.h"
#include "LayerTools.h"

namespace Core
{

QVector<TransferFunction*> HistogramRendererConfiguration::availableTransferFunctions = QVector<TransferFunction*>()
        << new TransferFunction()
        << new TransferFunction("Square", &pow2)
        << new TransferFunction("Log", &log);

QStringList HistogramRendererConfiguration::getAvailableTransferFunctionNames()
{
    QStringList list;
    foreach(TransferFunction * tf, availableTransferFunctions)
    {
        list << tf->getName();
    }
    return list;
}

TransferFunction * HistogramRendererConfiguration::getTransferFunctionByName(const QString &name)
{
    foreach(TransferFunction * tf, availableTransferFunctions)
    {
        if (tf->getName() == name) return tf;
    }
    return 0;
}


inline double clamp(double value, double vmin=0.0, double vmax=1.0)
{
    return (value >= vmax) ? vmax : (value < vmin) ? vmin : value;
}

inline double normalize(double value, double vmin, double vmax)
{
    return (value-vmin)/(vmax-vmin);
}

//******************************************************************************

HistogramLayerRenderer::HistogramLayerRenderer() :
    LayerRenderer()
{
}

//******************************************************************************

void HistogramLayerRenderer::setHistConfiguration(const HistogramRendererConfiguration & conf)
{
    _histConf = conf;
}

//******************************************************************************

/*!
 * \brief HistogramLayerRenderer::setupConfiguration method allows to configurate the renderer using the ImageLayer
 * \param ImageLayer
 * \return true if configuration is successful
 */
bool HistogramLayerRenderer::setupConfiguration(ImageLayer *layer)
{

    if (!LayerRenderer::setupConfiguration(layer))
        return false;

    computeQuantileMinMaxValues(layer, _settings.quantileMinValue, _settings.quantileMaxValue,
                                &_histConf.qMinValues, &_histConf.qMaxValues);

    int nbBands = layer->getNbBands();
    _histConf.transferFunctions.clear();
    _histConf.isDiscreteValues.clear();
    _histConf.normHistStops.clear();
    for (int i=0;i<nbBands;i++)
    {
        _histConf.transferFunctions << HistogramRendererConfiguration::availableTransferFunctions[0];
        _histConf.isDiscreteValues << false;
        double a=(_histConf.qMaxValues[i] - _histConf.qMinValues[i]) / (_conf.maxValues[i] - _conf.minValues[i]);
        double b=(_histConf.qMinValues[i] - _conf.minValues[i]) / (_conf.maxValues[i] - _conf.minValues[i]);
        _histConf.normHistStops << resetStops(nbBands == 1 ? -1 : i, a, b);
    }
    return true;

}

//******************************************************************************
/*!
  \bried HistogramLayerRenderer::render to render image data into RGB (24 bits) format.
  Histogram configuration has parameters :
    1) normalized gradient stops (correspond to sliders in the view)
    2) transfer function
    3) property if color are discrete
    4)

 */

inline void renderPixel(float * srcPtr, uchar *dstPtr, const QVector<int> &mapping,
                        const QVector<double> & minValues, const QVector<double> & maxValues,
                        const QVector<TransferFunction *> &transferFunctions, const QVector<bool> & isDiscreteValues,
                        const QVector<QGradientStops> & normHistStops);


cv::Mat HistogramLayerRenderer::render(const cv::Mat &rawData)
{

    cv::Mat outputImage8U;
    if (!checkBeforeRender(rawData))
        return outputImage8U;

    int w=rawData.cols;
    int h=rawData.rows;
    int nbBands = rawData.channels();
    outputImage8U=cv::Mat::zeros(h,w,CV_8UC3);

    // convert to 32F:
    cv::Mat rawData32F = rawData;
    if (rawData32F.depth() != CV_32F)
    {
        rawData32F.convertTo(rawData32F, CV_32F);
    }
    QVector<int> & mapping = _conf.toRGBMapping;
    float * srcPtr = reinterpret_cast<float*>(rawData32F.data);
    uchar * dstPtr = outputImage8U.data;


    const QVector<double> &minValues = _conf.minValues;
    const QVector<double> &maxValues = _conf.maxValues;
    const QVector<TransferFunction*> & transferFunctions = _histConf.transferFunctions;
    const QVector<bool> & isDiscreteValues = _histConf.isDiscreteValues;
    const QVector<QGradientStops> & normHistStops = _histConf.normHistStops;

    for (int i=0;i<w*h;i++)
    {
        renderPixel(srcPtr, dstPtr, mapping,
                    _conf.minValues, _conf.maxValues,
                    _histConf.transferFunctions, _histConf.isDiscreteValues,
                    _histConf.normHistStops);


        srcPtr+=nbBands;
        dstPtr+=3;
    }

    return outputImage8U;
}


#define ComputePixelValue(i, band) \
    index = mapping[i];                      \
    value = srcPtr[index];                  \
    a=minValues[index];       \
    b=maxValues[index];       \
    tf = transferFunctions[index];    \
    if (tf) \
    { \
        isDiscreteColors=isDiscreteValues[index];  \
        \
        /* Clamp value between band min/max and normalize between [0,1]*/ \
        value = normalize(clamp(value, a, b),a, b); \
        /* Apply transfer function and normalize value in [0,1] */ \
        value = tf->evaluate(value); \
        value = normalize(value, tf->evaluate(0.0), tf->evaluate(1.0)); \
        \
        l = normHistStops[index].size(); \
        fStop = normHistStops[index][0]; \
        lStop = normHistStops[index][l-1];    \
        if (value < fStop.first) \
        {   \
            dstPtr[i]+=fStop.second.band(); \
        }   \
        else if (value >= lStop.first)  \
        {   \
            dstPtr[i]+=lStop.second.band();   \
        }   \
        else    \
        {   \
            for (int j=0;j<l-1;j++) \
            {   \
                fStop = normHistStops[index][j];   \
                lStop = normHistStops[index][j+1]; \
                if (value >= fStop.first && value < lStop.first)    \
                {   \
                    dstPtr[i]+=fStop.second.band();    \
                    if (!isDiscreteColors)  \
                    {   \
                       alpha = (lStop.first - value)/(lStop.first - fStop.first);   \
                       dstPtr[i]*=alpha;    \
                       dstPtr[i]+=(1.0-alpha)*lStop.second.band(); \
                    }   \
                    break;  \
                }   \
            }   \
        } \
    }


inline void renderPixel(float * srcPtr, uchar * dstPtr, const QVector<int> & mapping,
                        const QVector<double> &minValues, const QVector<double> &maxValues,
                        const QVector<TransferFunction*> & transferFunctions, const QVector<bool> & isDiscreteValues,
                        const QVector<QGradientStops> & normHistStops)
{
    int index;
    double value = 0.0;
    double alpha = 0.0, a, b;
    TransferFunction * tf = 0;
    bool isDiscreteColors = false;
    int l = -1;
    QGradientStop fStop, lStop;
    // loop on input bands :
    dstPtr[0]=0;
    dstPtr[1]=0;
    dstPtr[2]=0;


    ComputePixelValue(0, red);
    ComputePixelValue(1, green);
    ComputePixelValue(2, blue);

}


//******************************************************************************

QGradientStops resetStops(int rgbBand, double a, double b)
{
    // number of stops for each band should be 2
    int n = (rgbBand < 0) ? 3 : 2;
    QGradientStops outputStops(n);
    for (int i=0;i<n;i++)
    {
        int c = qRound(255.*i/(n-1));
        QColor cc;
        if (rgbBand<0)
            cc = QColor(c, c, c);
        else if (rgbBand == 0)
            cc = QColor(c,0,0);
        else if (rgbBand == 1)
            cc = QColor(0,c,0);
        else if (rgbBand == 2)
            cc = QColor(0,0,c);
        else if (rgbBand > 2)
            cc = QColor(c,c,c);
        outputStops[i] = QGradientStop(i*1.0/(n-1)*a + b, cc);
    }
    return outputStops;
}

}
