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

struct ImageRendererConfiguration {
    QVector<int> toRGBMapping; //!< RGB to Bands mapping. Key is RGB and value is image band
    QVector<double> minValues;
    QVector<double> maxValues;
};

class GIV_DLL_EXPORT ImageRenderer : public QObject
{

    Q_OBJECT

public:
    ImageRenderer(QObject * parent = 0);
    virtual cv::Mat render(const cv::Mat & rawData, bool isBGRA=false);

    ImageRendererConfiguration getConfiguration() const
    { return _conf; }
    void setConfiguration(const ImageRendererConfiguration &conf);

    virtual bool setupConfiguration(ImageDataProvider * dataProvider);

protected:
    bool setToRGBMapping(ImageDataProvider * provider);
    inline bool checkBeforeRender(const cv::Mat &);

    ImageRendererConfiguration _conf;


};


//******************************************************************************

}

#endif // IMAGERENDERER_H
