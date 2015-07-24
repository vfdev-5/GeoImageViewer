#ifndef HISTOGRAMIMAGERENDERER_H
#define HISTOGRAMIMAGERENDERER_H


// Qt
#include <QGradientStops>

// Project
#include "LibExport.h"
#include "ImageRenderer.h"
#include "TransferFunctions.h"

namespace Core
{

class ImageDataProvider;

//******************************************************************************

struct GIV_DLL_EXPORT HistogramRendererConfiguration : public ImageRendererConfiguration
{
    QVector<double> qMinValues; //!< quantile min values at 2.5% of total histogram sum
    QVector<double> qMaxValues; //!< quantile max values at 97.5% of total histogram sum

    // Below parameters are used in rendering phase
    enum Mode {GRAY, RGB};
    Mode mode;
//    // GRAY mode parameters:
    QVector<bool> isDiscreteValues;
    QVector<TransferFunction*> transferFunctions;
    QVector<QGradientStops> normHistStops; //!< gradient stops are normalized due to transferFunctions

    HistogramRendererConfiguration() :
        mode(GRAY)
    {}

    virtual void copy(ImageRendererConfiguration * output);

    static QStringList getAvailableTransferFunctionNames();
    static TransferFunction* getTransferFunctionByName(const QString & name);
    static QVector<TransferFunction*> availableTransferFunctions;
};



class GIV_DLL_EXPORT HistogramImageRenderer : public ImageRenderer
{
    Q_OBJECT
public:

public:
    HistogramImageRenderer(QObject * parent = 0);
    virtual cv::Mat render(const cv::Mat & rawData, const ImageRendererConfiguration * conf, bool isBGRA=false);

    static bool setupConfiguration(const ImageDataProvider *dataProvider, HistogramRendererConfiguration * conf, HistogramRendererConfiguration::Mode mode);

    static HistogramRendererConfiguration::Mode getDefaultMode(const ImageDataProvider *dataProvider);

protected:
    bool checkBeforeRender(int nbBands, const HistogramRendererConfiguration * conf);

};

//******************************************************************************

inline bool HistogramImageRenderer::checkBeforeRender(int nbBands, const HistogramRendererConfiguration * conf)
{
    if (!ImageRenderer::checkBeforeRender(nbBands, conf))
        return false;

    if (conf->mode == HistogramRendererConfiguration::GRAY)
    {
        return !conf->normHistStops.isEmpty() &&
                !conf->isDiscreteValues.isEmpty() &&
                !conf->transferFunctions.isEmpty();
    }
    else if (conf->mode == HistogramRendererConfiguration::RGB)
    {
        return !conf->normHistStops.isEmpty();
    }
    return false;
}

//******************************************************************************

QGradientStops resetStops(int rgbBand=-1, double a=1.0, double b=0.0);
QGradientStops createStops(int nbStops, const QColor & startColor, const QColor & endColor, double startValue, double endValue);

}

#endif // HISTOGRAMIMAGERENDERER_H
