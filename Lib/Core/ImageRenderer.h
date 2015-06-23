#ifndef IMAGERENDERER_H
#define IMAGERENDERER_H

// Qt
#include <QObject>
#include <QVector>
#include <QMap>

// OpenCV
#include <opencv2/core/core.hpp>

// Project
#include "LibExport.h"

namespace Core
{

class ImageDataProvider;

//******************************************************************************

struct GIV_DLL_EXPORT ImageRendererConfiguration {
    QVector<int> toRGBMapping; //!< RGB to Bands mapping. Key is RGB and value is image band
    QVector<double> minValues;
    QVector<double> maxValues;

    virtual void copy(ImageRendererConfiguration * output)
    {
        if (!output || output == this)
            return;
        *output = *this;
    }

};

class GIV_DLL_EXPORT ImageRenderer : public QObject
{

    Q_OBJECT

public:
    ImageRenderer(QObject * parent = 0);
    virtual cv::Mat render(const cv::Mat & rawData, const ImageRendererConfiguration * conf, bool isBGRA=false);

    static bool setupConfiguration(const ImageDataProvider *dataProvider, ImageRendererConfiguration * conf);

protected:
    bool checkBeforeRender(int nbBands, const ImageRendererConfiguration * conf);

};

//******************************************************************************

inline bool ImageRenderer::checkBeforeRender(int nbBands, const ImageRendererConfiguration * conf)
{
    if (nbBands != conf->minValues.size() ||
            nbBands != conf->maxValues.size())
        return false;
    if (conf->toRGBMapping.size() != 3)
        return false;
    return true;
}

//******************************************************************************

QVector<int> computeToRGBMapping(const ImageDataProvider *provider);

//******************************************************************************

}

#endif // IMAGERENDERER_H
