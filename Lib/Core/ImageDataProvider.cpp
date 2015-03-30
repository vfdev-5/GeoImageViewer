
// STD
#include <cmath>
#include <limits>


// GDAL
#include <gdal_priv.h>

// Qt
#include <QString>
#include <QFileInfo>
#include <qmath.h>

// Project
#include "LayerUtils.h"
#include "ImageDataProvider.h"

namespace Core
{

const float ImageDataProvider::NoDataValue = -FLT_MAX + 1.0;

//******************************************************************************
/*!
  \class ImageDataProvider
  \brief abstract class that provides image data to other structures.
  Derived classes specifies the manner of how data is provided : from a GDAL Dataset, temp buffer, etc
  Class contains input data info which represents the original data (inputNbBands, inputIsComplex, etc)
  and provided data info which represents provided data type (which can be different from the input data).
  Thus, a ImageDataProvider can be seen as a filter.


    Option cutNoDataBRBoundary allows to get a matrix without bottom-right boundary from
    getImageData(QRect, 512, 512) even if size is specified.
    E.g. : user calls getImageData(r, 512, 512) and cutNoDataBRBoundary=true =>
    the output matrix will be smaller than 512, 512 if requested rect r when mapped
    to the output matrix zone is smaller than the output matrix rect.
    Default value of cutNoDataBRBoundary is true.



*/

//******************************************************************************

ImageDataProvider::ImageDataProvider(QObject *parent) :
    QObject(parent),
    _cutNoDataBRBoundary(true)
{
}

//******************************************************************************

void ImageDataProvider::setupDataInfo(const cv::Mat & src, ImageDataProvider * dst)
{
    dst->_nbBands   = src.channels();
    dst->_width     = src.cols;
    dst->_height    = src.rows;
    // The method returns the matrix element channel size in bytes, that is,
    // it ignores the number of channels. For example, if the matrix type is CV_16SC3 ,
    // the method returns sizeof(short) or 2.
    dst->_depth     = src.elemSize1();
    dst->_isComplex = false;
}

//******************************************************************************
//******************************************************************************

GDALDataProvider::GDALDataProvider(QObject *parent) :
    ImageDataProvider(parent)
{
}

//******************************************************************************

GDALDataProvider::~GDALDataProvider()
{
    if (_dataset)
        GDALClose(_dataset);
}

//******************************************************************************

bool GDALDataProvider::setup(const QString &filepath)
{
    _dataset = static_cast<GDALDataset *>(GDALOpen(filepath.toStdString().c_str(), GA_ReadOnly));
    if (!_dataset)
    {
        SD_TRACE( "GDALDataProvider::setup : Failed to open input file" )
        return false;
    }
    _filePath=filepath;

    _width = _inputWidth = _dataset->GetRasterXSize();
    _height = _inputHeight = _dataset->GetRasterYSize();
    _nbBands = _inputNbBands =_dataset->GetRasterCount();

    if (_width*_height*_nbBands <= 0)
    {
        SD_TRACE( "GDALDataProvider::setup : Image dimensions are not valid" );
        return false;
    }

    // pixel type:
    GDALDataType datatype = _dataset->GetRasterBand(1)->GetRasterDataType();
    _isComplex = _inputIsComplex = GDALDataTypeIsComplex(datatype) == 1;

    _inputDepth = GDALGetDataTypeSize(datatype)/8;
    _depth = 4;


    // setup band names :
    if (!_inputIsComplex)
    {
        for (int i=0;i<_nbBands;i++)
        {
            _bandNames << QObject::tr("band %1").arg(i+1);
        }
    }
    else
    {
        for (int i=0;i<_nbBands;i++)
        {
            _bandNames << QObject::tr("band %1 Re").arg(i+1);
            _bandNames << QObject::tr("band %1 Im").arg(i+1);
            _bandNames << QObject::tr("band %1 Abs").arg(i+1);
            _bandNames << QObject::tr("band %1 Phase").arg(i+1);
        }
        _nbBands *= 4;
        _isComplex = false;
    }

    // Geo info :
    _pixelExtent = QRect(0,0,_width, _height);

    return true;
}

//******************************************************************************
/*!
    Method to get image data as cv::Mat.
    Input :
        - srcPixelExtent, source pixel extent of the image, e.g. rect(20,30|w=50,h=60) of the image
        - dstPixelWidth and dstPixelHeight, output image size in pixels

    Output cv::Mat has 32F depth of single band pixel

*/
cv::Mat GDALDataProvider::getImageData(const QRect & srcPixelExtent, int dstPixelWidth, int dstPixelHeight) const
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

    // Define rectangle in the output matrix where the data will be written:
    cv::Rect r(qFloor(scaleX*(srcRequestedExtent.x() - srcExtent.x())),
               qFloor(scaleY*(srcRequestedExtent.y() - srcExtent.y())),
               reqScaledW,
               reqScaledH);

    // Output matrix is encoded as Float32 for each band (Real Imagery) or Float32 for (re,im,abs,phase) and for each band (Complex Imagery)
    // Matrix is initialized with noData value : -FLT_MAX + 1.0
    out = cv::Mat(dstPixelExtent.height(),
                  dstPixelExtent.width(),
                  convertTypeToOpenCV(4, false, _nbBands),
                  cv::Scalar(NoDataValue));
    cv::Mat b = out(r);

    // Read data:
    GDALDataType dstDatatype= (!_isComplex) ? GDT_Float32 : GDT_CFloat32;
    int depth = (!_isComplex) ? GDALGetDataTypeSize(dstDatatype)/8 : GDALGetDataTypeSize(dstDatatype)/4; // REAL -> {32F} | CMPLX -> {32F(re),32F(im),32F(abs),32F(phase)}
    int nbBands = _dataset->GetRasterCount();

//    int desiredNbOfSamples=srcImgExtent.width()*srcImgExtent.height();

    for (int i=0; i<nbBands; i++)
    {
//        GDALRasterBand * band = _dataset->GetRasterBand(i+1)->GetRasterSampleOverview(desiredNbOfSamples);
//        scaleX = band->GetXSize()*1.0/_pixelExtent.width();
//        scaleY = band->GetYSize()*1.0/_pixelExtent.height();

        GDALRasterBand * band = _dataset->GetRasterBand(i+1);

        CPLErr err = band->RasterIO( GF_Read,
                                     srcRequestedExtent.x(),
                                     srcRequestedExtent.y(),
                                     srcRequestedExtent.width(),
                                     srcRequestedExtent.height(),
                                     b.data+i*depth, // data buffer offset : CMPLX -> {32F(re),32F(im),32F(abs),32F(phase)}
                                     r.width, // data buffer size (width)
                                     r.height,// data buffer size (height)
                                     dstDatatype, // pixel type : REAL -> 32F | CMPLX -> 64F
                                     nbBands*depth, // pixel size : REAL -> nbBands * 4 {32F} | CMPLX -> nbBands * 8 = {32F(re),32F(im),32F(abs),32F(phase)} * nbBands
                                     nbBands*depth*dstPixelExtent.width()); // buffer line length

        if (err != CE_None)
        {
            SD_TRACE( "Failed to read data" );
            return cv::Mat();
        }

        if (_isComplex)
        {
            // Compute Abs,Phase and write into the buffer
            uchar * srcPtr = b.data+i*depth;
            double re, im, abs, phase;
            for (int i=0;i<r.width*r.height;i++)
            {
                re = reinterpret_cast<float*>(srcPtr)[0];
                im = reinterpret_cast<float*>(srcPtr)[1];
                abs = qSqrt(re*re + im*im);
                phase = atan2(im,re);
                reinterpret_cast<float*>(srcPtr)[2] = (float) abs;
                reinterpret_cast<float*>(srcPtr)[3] = (float) phase;
                srcPtr+=depth*nbBands;
            }
        }

    }

    return out;
}

//******************************************************************************

QString GDALDataProvider::fetchProjectionRef() const
{
    return _dataset->GetProjectionRef();
}

//******************************************************************************

QPolygonF GDALDataProvider::fetchGeoExtent(const QRect &pixelExtent) const
{
    return computeGeoExtent(_dataset, pixelExtent);
}

//******************************************************************************

QVector<double> GDALDataProvider::fetchGeoTransform() const
{
    QVector<double> out(6);
    _dataset->GetGeoTransform(out.data());
    return out;
}

//******************************************************************************

}
