
// Project
#include "HistogramImageRenderer.h"
#include "ImageDataProvider.h"
#include "LayerUtils.h"

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

/*!
    \class HistogramRendererConfiguration
    \brief implements the configuration of the HistogramImageRenderer.
    Class contains following parameters :
    - mode          : rendering configuration mode (gray or rgb)
    - normHistStops : N arrays of gray mode 'stops' (a 'stop' is a pair of 'normalized value in [0,1]' and 'RGB color')
                      which are used to map input image data value into an RGB color. The variable N corresponds to
                      the number of channels in the input image. Gray mode means that a selected one band of the input image
                      is processed as a gray image and then rendered to RGB colors. Number of 'stops' is not limited.
                      For example, when 'stops' are defined as { (0.0, black), (0.5, red), (1.0, white) }, then
                      input image values (v in [min, max]) are mapped as following:
                        a) v is clamped and then normalized between [0, 1] using input min/max values
                        b) if v = 0.0 then output RGB color is black due to the value of the first 'stop'.
                        c) if v is between 0.0 and 0.5 (value of the 2nd stop) then output RGB color is computed
                            as an interpolation between 1st stop color and 2nd stop color.
                        d) if v is between 0.5 and 1.0 (value of the 3rd stop) the output RGB color is computed
                            similarly to the case c).
                        e) if v = 1.0 then the output RGB color is white due to the value of the last 'stop'.

    - transferFunctions : N transfer functions which used to transform the input image value before the mapping to RGB color.

    - isDiscreteValues : N boolean values used to indicate whether there is a gradient between to colors of the 'stops'. This
                         conditions appears when RGB color is computed as an interpolation between to neighbours colors.

    - normRGBHistStops : 3 arrays of RGB mode 'stops' which are used to map input data values to RGB colors.
                         The difference from normHistStops is that there are only 2 'stops' possible for each red,
                         green and blue channels. Precisely, the first array contains 2 stops between black and red.
                         The second array contains 2 stops between black and green. The third array contains 2 stops
                         between black and blue.


    \class HistogramImageRenderer
    \brief is derived from ImageRenderer and implements image rendering using band histograms.
    The configuration is set using class HistogramRendererConfiguration.
    The renderer has two modes : gray and rgb
    There are following cases managed when input image is :
    1) 1 band and not complex then the HistogramRendererConfiguration is
        a) mode is GRAY only
        b) normHistStops is one array of default 'stops'. The number 'stops' and possible colors are not limited.
        c) normRGBHistStops is empty

    2) M bands, complex imagery then the HistogramRendererConfiguration is
        a) mode is GRAY only
        b) normHistStops is M arrays of default 'stops'. The number 'stops' and possible colors are not limited.
        c) normRGBHistStops is empty

    3) N Bands, not complex imagery then the HistogramRendererConfiguration is
        a) mode is GRAY or RGB
        b) normHistStops is N arrays of default 'stops'. The number 'stops' and possible colors are not limited.
        c) normRGBHistStops is 3 arrays of default 'stops'





 */

//******************************************************************************

HistogramImageRenderer::HistogramImageRenderer(QObject *parent) :
    ImageRenderer(parent)
{
}

//******************************************************************************

void HistogramImageRenderer::setHistConfiguration(const HistogramRendererConfiguration & conf)
{
    _histConf = conf;
}

//******************************************************************************

/*!
 * \brief HistogramImageRenderer::setupConfiguration method allows to configurate the renderer using the ImageDataProvider
 * \param ImageDataProvider
 * \return true if configuration is successful
 */
bool HistogramImageRenderer::setupConfiguration(ImageDataProvider * provider)
{

    if (!ImageRenderer::setupConfiguration(provider))
        return false;

    computeQuantileMinMaxValues(provider->getMinValues(), provider->getMaxValues(),
                                provider->getBandHistograms(),
                                _settings.quantileMinValue, _settings.quantileMaxValue,
                                &_histConf.qMinValues, &_histConf.qMaxValues);

    int nbBands = provider->getNbBands();
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

bool HistogramImageRenderer::chechHistConf()
{
    return !_histConf.normHistStops.isEmpty() &&
            !_histConf.qMinValues.isEmpty() &&
            _histConf.qMinValues.size() == _histConf.qMaxValues.size() &&
            !_histConf.isDiscreteValues.isEmpty() &&
            !_histConf.transferFunctions.isEmpty();
}

//******************************************************************************
/*!
  \brief HistogramImageRenderer::render to render image data into RGB (24 bits) format.
  Histogram configuration has parameters :
    1) normalized gradient stops (correspond to sliders in the view)
    2) transfer function
    3) property if color are discrete
    4)

   \return Matrix in RGBA 32-bits format, 4 channels

 */

inline void renderPixel(float * srcPtr, uchar *dstPtr, const QVector<int> &mapping,
                        const QVector<double> & minValues, const QVector<double> & maxValues,
                        const QVector<TransferFunction *> &transferFunctions, const QVector<bool> & isDiscreteValues,
                        const QVector<QGradientStops> & normHistStops);


cv::Mat HistogramImageRenderer::render(const cv::Mat &rawData, bool isBGRA)
{

    cv::Mat outputImage8U;
    if (!checkBeforeRender(rawData) || !chechHistConf())
        return outputImage8U;

    int w=rawData.cols;
    int h=rawData.rows;
    int nbBands = rawData.channels();
    outputImage8U=cv::Mat::zeros(h,w,CV_8UC4);

    // Get alpha channel and rewrite noDataValues to zero
    //    cv::Mat mask, alpha8U = oRawData > ImageDataProvider::NoDataValue;
    //    alpha.convertTo(mask, oRawData.depth());
    //    cv::Mat rawData = oRawData.mul(mask);
    //    std::vector<cv::Mat> iAlpha8U(nbBands);
    //    cv::split(alpha8U, &iAlpha8U[0]);

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

    float a32 = 0.0;
    uchar alpha = 255;
    for (int i=0;i<w*h;i++)
    {
        // get alpha channel and rewrite noDataValues to zero
        a32 = qMin(qMin(srcPtr[mapping[0]], srcPtr[mapping[1]]), srcPtr[mapping[2]]);
        if (a32 > ImageDataProvider::NoDataValue)
        {
            alpha = 255;
        }
        else
        {
            alpha = 0;
            srcPtr[mapping[0]] = 0.0;
            srcPtr[mapping[1]] = 0.0;
            srcPtr[mapping[2]] = 0.0;
        }

        // render to RGB
        renderPixel(srcPtr, dstPtr, mapping,
                    _conf.minValues, _conf.maxValues,
                    _histConf.transferFunctions, _histConf.isDiscreteValues,
                    _histConf.normHistStops);

        if (isBGRA)
            std::swap(dstPtr[0], dstPtr[2]);

        // set alpha channel value:
        dstPtr[3] = alpha;

        srcPtr+=nbBands;
        dstPtr+=4;
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
    dstPtr[i]=fStop.second.band(); \
}   \
    else if (value >= lStop.first)  \
{   \
    dstPtr[i]=lStop.second.band();   \
}   \
    else    \
{   \
    for (int j=0;j<l-1;j++) \
{   \
    fStop = normHistStops[index][j];   \
    lStop = normHistStops[index][j+1]; \
    if (value >= fStop.first && value < lStop.first)    \
{   \
    double nvalue = fStop.second.band(); /*dstPtr[i]+=fStop.second.band();*/    \
    if (!isDiscreteColors)  \
{   \
    alpha = (lStop.first - value)/(lStop.first - fStop.first);   \
    nvalue*=alpha; /*dstPtr[i]*=alpha;*/    \
    nvalue+=(1.0-alpha)*lStop.second.band(); /*dstPtr[i]+=(1.0-alpha)*lStop.second.band();*/ \
}   \
    dstPtr[i] = (uchar) nvalue; \
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




#ifdef _DEBUG
    index = mapping[0];
    value = srcPtr[index];
    a=minValues[index];
    b=maxValues[index];
    tf = transferFunctions[index];
    if (tf)
    {
        isDiscreteColors=isDiscreteValues[index];

        /* Clamp value between band min/max and normalize between [0,1]*/
        value = normalize(clamp(value, a, b),a, b);
        /* Apply transfer function and normalize value in [0,1] */
        value = tf->evaluate(value);
        value = normalize(value, tf->evaluate(0.0), tf->evaluate(1.0));

        l = normHistStops[index].size();
        fStop = normHistStops[index][0];
        lStop = normHistStops[index][l-1];
        if (value < fStop.first)
        {
            dstPtr[0]=fStop.second.red();
        }
        else if (value >= lStop.first)
        {
            dstPtr[0]=lStop.second.red();
        }
        else
        {
            for (int j=0;j<l-1;j++)
            {
                fStop = normHistStops[index][j];
                lStop = normHistStops[index][j+1];
                if (value >= fStop.first && value < lStop.first)
                {
//                    dstPtr[0]+=fStop.second.red();
                    double nvalue = fStop.second.red();
                    if (!isDiscreteColors)
                    {
                        alpha = (lStop.first - value)/(lStop.first - fStop.first);
                        nvalue*=alpha;
                        nvalue+=(1.0-alpha)*lStop.second.red();
                    }
                    dstPtr[0] = (uchar) (nvalue);
                    break;
                }
            }
        }
    }
#else
    ComputePixelValue(0, red);
#endif
    ComputePixelValue(1, green);
    ComputePixelValue(2, blue);

#ifdef _DEBUG
    int rr = dstPtr[0];
    int gg = dstPtr[1];
    int bb = dstPtr[2];
#endif


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
