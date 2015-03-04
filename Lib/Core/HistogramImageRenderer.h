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

struct GIV_DLL_EXPORT HistogramRendererConfiguration
{

    QVector<double> qMinValues; //!< quantile min values at 2.5% of total histogram sum
    QVector<double> qMaxValues; //!< quantile max values at 97.5% of total histogram sum

    // Below parameters are used in rendering phase
    enum Mode {GRAY, RGB};
    Mode mode;
    // GRAY mode parameters:
    QVector<bool> isDiscreteValues;
    QVector<TransferFunction*> transferFunctions;
    QVector<QGradientStops> normHistStops; //!< gradient stops are normalized due to transferFunctions

    // RGB mode parameters:
    QVector<QGradientStops> normRGBHistStops; //!< gradient stops are normalized due to transferFunctions
    TransferFunction * rgbTransferFunction;
    bool isRGBDiscreteValue;


    HistogramRendererConfiguration() :
        mode(GRAY),
        rgbTransferFunction(0),
        isRGBDiscreteValue(false)
    {}

    static QStringList getAvailableTransferFunctionNames();
    static TransferFunction* getTransferFunctionByName(const QString & name);
    static QVector<TransferFunction*> availableTransferFunctions;
};

class GIV_DLL_EXPORT HistogramImageRenderer : public ImageRenderer
{
    Q_OBJECT
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
    HistogramImageRenderer(QObject * parent = 0);
    virtual cv::Mat render(const cv::Mat & rawData, bool isBGRA=false);
    virtual bool setupConfiguration(ImageDataProvider * provider);

    HistogramRendererConfiguration getHistConfiguration() const
    { return _histConf; }
    void setHistConfiguration(const HistogramRendererConfiguration &conf);

protected:

    inline bool checkHistConf();

    HistogramRendererConfiguration _histConf;
    Settings _settings;


};

//******************************************************************************

QGradientStops resetStops(int rgbBand=-1, double a=1.0, double b=0.0);

}

#endif // HISTOGRAMIMAGERENDERER_H
