#ifndef IMAGELAYER_H
#define IMAGELAYER_H


// Qt
#include <QRect>
#include <QRectF>
#include <QPolygonF>
#include <QImage>
#include <QStringList>

// OpenCV
#include <opencv2/core/core.hpp>

// Project
#include "Global.h"

class GDALDataset;

namespace Core
{

//******************************************************************************


class ImageLayer
{
    // Dataset file is used (e.g. HDF5 imagery subset name is different from original image file)
    PROPERTY_GETACCESSOR(QString, filePath, getFilePath)
    // Layer info (e.g nbBands = 4*Original Nb Bands for Complex images, depth is always 32F )
    PROPERTY_GETACCESSOR(int, nbBands, getNbBands)

    PROPERTY_GETACCESSOR(bool, isComplex, isComplex)

    PROPERTY_GETACCESSOR(int, width, getWidth)
    PROPERTY_GETACCESSOR(int, height, getHeight)

    PROPERTY_GETACCESSOR(QRect, pixelExtent, getPixelExtent)

    PROPERTY_ACCESSORS(QVector<double>, minValues, getMinValues, setMinValues)
    PROPERTY_ACCESSORS(QVector<double>, maxValues, getMaxValues, setMaxValues)
    PROPERTY_GETACCESSOR(QVector<QVector<double> >, bandHistograms, getBandHistograms)
    PROPERTY_GETACCESSOR(QStringList, bandNames, getBandNames)


public:
    ImageLayer();
    ~ImageLayer();
    bool setup(const QString & filepath);

    cv::Mat getImageData(const QRect & srcPixelExtent=QRect(), int dstPixelWidth=0, int dstPixelHeight=0);

    void setBandHistogram(int bandIndex, const QVector<double> & bandHistogram) //!< Histogram should be normalized
    {
        if (bandIndex == _bandHistograms.size())
        {
            _bandHistograms << bandHistogram;
        }
        else if (bandIndex >= 0 && bandIndex < _bandHistograms.size())
        {
            _bandHistograms[bandIndex] = bandHistogram;
        }
    }

    GDALDataset * getDataset()
    { return _dataset; }

protected:

    GDALDataset * _dataset;

};

//******************************************************************************

}

#endif // IMAGELAYER_H
