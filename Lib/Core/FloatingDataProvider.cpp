
// Qt
#include <qmath.h>

// OpenCV
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "FloatingDataProvider.h"
#include "LayerUtils.h"

namespace Core
{

//******************************************************************************

FloatingDataProvider::FloatingDataProvider(QObject *parent) :
    ImageDataProvider(parent)
{
}

//******************************************************************************

cv::Mat FloatingDataProvider::getImageData(const QRect & srcPixelExtent, int dstPixelWidth, int dstPixelHeight) const
{
    cv::Mat out;

    QRect srcRequestedExtent, srcExtent;
    // If source pixel extent is not specified -> take the whole image pixel extent
    if (srcPixelExtent.isEmpty())
    {
        srcRequestedExtent = _pixelExtent;
        srcExtent = _pixelExtent;
    }
    else
    {
        srcRequestedExtent = _pixelExtent.intersected(srcPixelExtent);
        srcExtent = srcPixelExtent;
    }

    if (srcRequestedExtent.isEmpty())
        return out;

    // If destination pixel extent is not specified -> take extent equals source extent <=> full resolution
    QSize dstPixelExtent;
    if (dstPixelWidth == 0 && dstPixelHeight == 0)
    { // Full resolution
        dstPixelExtent = srcRequestedExtent.size();
    }
    else if (dstPixelHeight == 0)
    { // output keeps aspect ratio
        int h = dstPixelWidth * srcExtent.height() * 1.0 / srcExtent.width();
        dstPixelExtent = QSize(dstPixelWidth, h);
    }
    else
    {
        dstPixelExtent = QSize(dstPixelWidth, dstPixelHeight);
    }

    double scaleX = dstPixelExtent.width() * 1.0 / srcExtent.width();
    double scaleY = dstPixelExtent.height() * 1.0 / srcExtent.height();

    int reqScaledW = qMin(qCeil(scaleX*srcRequestedExtent.width()), dstPixelExtent.width());
    int reqScaledH = qMin(qCeil(scaleY*srcRequestedExtent.height()), dstPixelExtent.height());

    if (_cutNoDataBRBoundary)
    {
        if (reqScaledW < dstPixelExtent.width())
            dstPixelExtent.setWidth(reqScaledW);
        if (reqScaledH < dstPixelExtent.height())
            dstPixelExtent.setHeight(reqScaledH);
    }

    cv::Rect r(qFloor(scaleX*(srcRequestedExtent.x() - srcExtent.x())),
               qFloor(scaleY*(srcRequestedExtent.y() - srcExtent.y())),
               reqScaledW,
               reqScaledH);

    // Output matrix is encoded as Float32 for each band (Real Imagery) or Float32 for (re,im,abs,phase) and for each band (Complex Imagery)
    out = cv::Mat(dstPixelExtent.height(),
                  dstPixelExtent.width(),
                  convertTypeToOpenCV(4, false, _nbBands),
                  cv::Scalar(NoDataValue));
    cv::Mat dstMat = out(r);


    // Read data:
    cv::Rect r2(srcRequestedExtent.x(), srcRequestedExtent.y(),
                srcRequestedExtent.width(), srcRequestedExtent.height());
    cv::Mat srcMat = _data(r2);
    cv::resize(srcMat, dstMat, dstMat.size());

//    displayMat(out, true, "out");

    return out;

}

//******************************************************************************

FloatingDataProvider* FloatingDataProvider::createDataProvider(const ImageDataProvider * src, const QRect & pixelExtent)
{

    FloatingDataProvider * dst = 0;
    QRect srcPixelExtent = src->getPixelExtent();
    QRect intersection = srcPixelExtent.intersected(pixelExtent);
    if (intersection.isEmpty())
        return dst;


    SD_TRACE(QString("Intersection : %1, %2 | %3, %4")
             .arg(pixelExtent.x())
             .arg(pixelExtent.y())
             .arg(pixelExtent.width())
             .arg(pixelExtent.height()));

    dst = new FloatingDataProvider();

    dst->setImageName("Region of " + src->getImageName());
    // Copy input info :
    dst->_inputWidth     = src->getWidth();
    dst->_inputHeight    = src->getHeight();
    dst->_inputDepth     = src->getDepthInBytes();
    dst->_inputNbBands   = src->getNbBands();
    dst->_inputIsComplex = src->isComplex();

    dst->_bandNames = src->getBandNames();

    dst->_data = src->getImageData(intersection);

    setupDataInfo(dst->_data, dst);

    dst->_pixelExtent = QRect(0,0,intersection.width(),intersection.height());

//    displayMat(dst->_data, true, "dst->_data");

    // compute data stats:
    if (!computeNormalizedHistogram(dst->_data,
                                    dst->_minValues,
                                    dst->_maxValues,
                                    dst->_bandHistograms,
                                    1000))
    {
        SD_TRACE("createDataProvider : Failed to compute image stats");
        delete dst;
        return 0;
    }


    return dst;
}

//******************************************************************************

}
