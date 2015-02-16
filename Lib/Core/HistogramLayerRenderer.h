#ifndef HISTOGRAMLAYERRENDERER_H
#define HISTOGRAMLAYERRENDERER_H


// Qt
#include <QGradientStops>

// Project
#include "LibExport.h"
#include "LayerRenderer.h"
#include "TransferFunctions.h"

namespace Core
{

//******************************************************************************

struct HistogramRendererConfiguration
{

    QVector<double> qMinValues; //!< quantile min values at 2.5% of total histogram sum
    QVector<double> qMaxValues; //!< quantile max values at 97.5% of total histogram sum

    // Below parameters are used in rendering phase
    QVector<bool> isDiscreteValues;
    QVector<TransferFunction*> transferFunctions;
    QVector<QGradientStops> normHistStops; //!< gradient stops are normalized due to transferFunctions


    HistogramRendererConfiguration()
    {}

    static QStringList getAvailableTransferFunctionNames();
    static TransferFunction* getTransferFunctionByName(const QString & name);
    static QVector<TransferFunction*> availableTransferFunctions;
};

class HistogramLayerRenderer : public LayerRenderer
{

public:

    struct Settings
    {
        double quantileMinValue;
        double quantileMaxValue;

        Settings() :
            quantileMinValue(2.5),
            quantileMaxValue(97.5)
        {
        }
    };

public:
    HistogramLayerRenderer();
    virtual cv::Mat render(const cv::Mat & rawData);
    virtual bool setupConfiguration(ImageLayer * layer);

    HistogramRendererConfiguration getHistConfiguration() const
    { return _histConf; }
    void setHistConfiguration(const HistogramRendererConfiguration &conf);



protected:

    HistogramRendererConfiguration _histConf;
    Settings _settings;


};

//******************************************************************************

QGradientStops resetStops(int rgbBand=-1, double a=1.0, double b=0.0);

}

#endif // HISTOGRAMLAYERRENDERER_H
