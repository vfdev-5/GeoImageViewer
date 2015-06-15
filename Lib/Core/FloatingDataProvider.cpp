
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

    if (srcPixelExtent.isEmpty() && dstPixelWidth == 0 && dstPixelHeight == 0)
    {
        return _data;
    }

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
//        dstPixelExtent = srcRequestedExtent.size();
        dstPixelExtent = srcExtent.size();
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

    if (_cutNoDataBRBoundary &&
            (srcRequestedExtent.x() == srcExtent.x()) &&
            (srcRequestedExtent.y() == srcExtent.y()))
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
//    out = cv::Mat(dstPixelExtent.height(),
//                  dstPixelExtent.width(),
//                  convertTypeToOpenCV(4, false, _nbBands),
//                  cv::Scalar::all(NoDataValue));
    out = cv::Mat(dstPixelExtent.height(),
                  dstPixelExtent.width(),
                  convertTypeToOpenCV(4, false, _nbBands));
    out.setTo(NoDataValue);

    cv::Mat dstMat = out(r);

    // Read data:
    cv::Rect r2(srcRequestedExtent.x(), srcRequestedExtent.y(),
                srcRequestedExtent.width(), srcRequestedExtent.height());
    cv::Mat srcMat = _data(r2);
    cv::resize(srcMat, dstMat, dstMat.size());

    return out;

}

//******************************************************************************

void FloatingDataProvider::setImageData(const QPoint &offset, const cv::Mat &data)
{
    QRect r(offset.x(), offset.y(), data.cols, data.rows);
    // if rect is in image:
    QRect imRect(0,0,_data.cols,_data.rows);
    r = imRect.intersected(r);
    if (r.isEmpty())
        return;

    cv::Mat dataCP;
    if (data.depth() != _data.depth())
    {
        data.convertTo(dataCP, _data.depth());
    }
    else
    {
        dataCP = data;
    }
    dataCP.copyTo(_data(cv::Rect(r.x(),r.y(),r.width(),r.height())));

    // call 'update' on current zone -> GeoImageItem should be updated
    emit dataChanged(QRect(offset.x(), offset.y(), data.cols, data.rows));
}

//******************************************************************************

bool FloatingDataProvider::create(const QString &name, const cv::Mat &src, const QRect &iIntersection)
{
    QRect intersection;
    if (iIntersection.isEmpty())
    {
        intersection = QRect(0,0,src.cols,src.rows);
    }
    else
    {
        intersection = QRect(0,0,src.cols,src.rows).intersected(iIntersection);
        if (intersection.isEmpty())
            return false;
    }

    setImageName(name);
    // Copy input info :
    _inputWidth     = src.cols;
    _inputHeight    = src.rows;
    _inputDepth     = src.elemSize1();
    _inputNbBands   = src.channels();
    _inputIsComplex = false;

    _bandNames.clear();
    for (int i = 0; i<src.channels();i++)
    {
        _bandNames << QString("band %1").arg(i+1);
    }

    cv::Rect r(intersection.x(), intersection.y(),
               intersection.width(), intersection.height());

    src(r).copyTo(_data);
    if (_data.depth() != CV_32F)
        _data.convertTo(_data, CV_32F);

//    displayMat(dst->_data, true, "dst->_data");

    setupDataInfo(_data, this);

    _pixelExtent = QRect(0,0,intersection.width(),intersection.height());

    // compute mask :
    cv::Mat mask = ImageDataProvider::computeMask(_data);

    // compute data stats:
    if (!computeNormalizedHistogram(_data, mask,
                                    _minValues,
                                    _maxValues,
                                    _bandHistograms,
                                    1000))
    {
        SD_TRACE("createDataProvider : Failed to compute image stats");
        return false;
    }
    return true;

}

//******************************************************************************

FloatingDataProvider* FloatingDataProvider::createEmptyDataProvider(const QString & name, int width, int height)
{
    FloatingDataProvider * dst = new FloatingDataProvider();

    cv::Mat src(height,
                width,
                CV_32F,
//                cv::Scalar::all(NoDataValue));
                cv::Scalar::all(10));
    src.at<float>(0,0) = 0;

    if (!dst->create(name, src, QRect()))
    {
        delete dst;
        return 0;
    }
    dst->setEditable(true);
    return dst;
}

//******************************************************************************

FloatingDataProvider* FloatingDataProvider::createDataProvider(const QString & name, const cv::Mat & src, const QRect & iIntersection)
{
    FloatingDataProvider * dst = new FloatingDataProvider();
    if (!dst->create(name, src, iIntersection))
    {
        delete dst;
        return 0;
    }
    return dst;
}

//******************************************************************************

FloatingDataProvider* FloatingDataProvider::createDataProvider(const ImageDataProvider * src, const QRect & iIntersection)
{
    FloatingDataProvider * dst = 0;
    QRect intersection = src->getPixelExtent().intersected(iIntersection);
    if (intersection.isEmpty())
        return dst;


    // test buffer size needed : if larger than 200 Mb -> error
    double size = intersection.width() * intersection.height() * src->getInputNbBands() * src->getInputDepthInBytes();
    if (size > 1024.0 * 1024.0 * 200.0 )
    {
        SD_ERR("Image selection is too large.");
        return dst;
    }

    dst = new FloatingDataProvider();

    dst->setImageName(tr("Region of ") + src->getImageName());
    // Copy input info which should be the same as src input info
    // output data provider is ROI of src and not a filter
    dst->_inputWidth     = src->getWidth();
    dst->_inputHeight    = src->getHeight();
    dst->_inputDepth     = src->getInputDepthInBytes();
    dst->_inputNbBands   = src->getInputNbBands();
    dst->_inputIsComplex = src->inputIsComplex();

    dst->_bandNames = src->getBandNames();

    dst->_data = src->getImageData(intersection);

    setupDataInfo(dst->_data, dst);

    dst->_pixelExtent = QRect(0,0,intersection.width(),intersection.height());

    // compute mask
    cv::Mat mask = ImageDataProvider::computeMask(dst->_data);

    // compute data stats:
    if (!computeNormalizedHistogram(dst->_data, mask,
                                    dst->_minValues,
                                    dst->_maxValues,
                                    dst->_bandHistograms,
                                    1000))
    {
        SD_TRACE("createDataProvider : Failed to compute image stats");
        delete dst;
        return 0;
    }

    // setup geo info
    dst->_projectionRef = src->fetchProjectionRef();
    dst->_geoExtent = src->fetchGeoExtent(intersection);
    dst->_geoTransform = Core::computeGeoTransform(dst->_geoExtent, intersection);

    return dst;
}

//******************************************************************************

QPolygonF FloatingDataProvider::fetchGeoExtent(const QRect &pixelExtent) const
{
    if (pixelExtent.isEmpty())
    {
        return _geoExtent;
    }
    else
    {
        // compute geo extent for given pixelExtent :
        return QPolygonF();
    }
}

//******************************************************************************

}
