#ifndef LAYERRENDERER_H
#define LAYERRENDERER_H

// Qt
#include <QVector>
#include <QMap>

// OpenCV
#include <opencv2/core/core.hpp>

namespace Core
{

//******************************************************************************

class ImageLayer;

struct LayerRendererConfiguration {

//    QMap<int,int> toRGBMapping; //!< RGB to Bands mapping. Key is RGB and value is image band
    QVector<int> toRGBMapping; //!< RGB to Bands mapping. Key is RGB and value is image band
    QVector<double> minValues;
    QVector<double> maxValues;
};

class LayerRenderer
{
public:
    LayerRenderer();
    virtual cv::Mat render(const cv::Mat & rawData);

    LayerRendererConfiguration getConfiguration() const
    { return _conf; }
    void setConfiguration(const LayerRendererConfiguration &conf);

    virtual bool setupConfiguration(ImageLayer * layer);

protected:

    bool setToRGBMapping(ImageLayer * layer);
    bool checkBeforeRender(const cv::Mat &);

    LayerRendererConfiguration _conf;


};


//******************************************************************************

}

#endif // LAYERRENDERER_H
