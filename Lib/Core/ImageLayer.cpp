
// STD
#include <cmath>

// GDAL
#include <gdal_priv.h>

// Qt
#include <QString>
#include <qmath.h>

// Project
#include "Global.h"
#include "LayerTools.h"
#include "ImageLayer.h"

namespace Core
{

//******************************************************************************
/*!
 * \class ImageLayer
 * \brief This class is used to
 */
//******************************************************************************

ImageLayer::ImageLayer() :
    _dataset(0)
{
}

//******************************************************************************

ImageLayer::~ImageLayer()
{
    if (_dataset)
        GDALClose(_dataset);
}

//******************************************************************************

bool ImageLayer::setup(const QString &filepath)
{

    _dataset = static_cast<GDALDataset *>(GDALOpen(filepath.toStdString().c_str(), GA_ReadOnly));
    if (!_dataset)
    {
        SD_TRACE( "ImageLayer::setup : Failed to open input file" )
        return false;
    }
    _filePath=filepath;

    _width = _dataset->GetRasterXSize();
    _height = _dataset->GetRasterYSize();
    _nbBands=_dataset->GetRasterCount();

    if (_width*_height*_nbBands <= 0)
    {
        SD_TRACE( "Image dimensions are not valid" );
        return false;
    }

    // pixel type:
    GDALDataType datatype = _dataset->GetRasterBand(1)->GetRasterDataType();
    _isComplex = GDALDataTypeIsComplex(datatype) == 1;

    // setup band names :
    if (!_isComplex)
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


cv::Mat ImageLayer::getImageData(const QRect &srcPixelExtent, int dstPixelWidth, int dstPixelHeight)
{
    cv::Mat out;
    QRect srcImgExtent, srcExtent;
    // If source pixel extent is not specified -> take the whole image pixel extent
    if (srcPixelExtent.isEmpty())
    {
        srcImgExtent = _pixelExtent;
        srcExtent = _pixelExtent;
    }
    else
    {
        srcImgExtent = _pixelExtent.intersected(srcPixelExtent);
        srcExtent = srcPixelExtent;
    }

    if (srcImgExtent.isEmpty())
        return out;

    // If destination pixel extent is not specified -> take extent equals source extent <=> full resolution
    QSize dstPixelExtent;
    if (dstPixelWidth == 0 && dstPixelHeight == 0)
    { // Full resolution
        dstPixelExtent = srcImgExtent.size();
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
    cv::Rect r(0, 0, scaleX*srcImgExtent.width(), scaleY*srcImgExtent.height());

    // Output matrix is encoded as Float32 for each band (Real Imagery) or Float32 for (re,im,abs,phase) and for each band (Complex Imagery)
    out = cv::Mat(dstPixelExtent.height(),
                  dstPixelExtent.width(),
                  convertTypeToOpenCV(4, false, _nbBands),
                  cv::Scalar(0));
    cv::Mat b = out(r);


    // Read data:
    GDALDataType dstDatatype= (!_isComplex) ? GDT_Float32 : GDT_CFloat32;
    int depth = (!_isComplex) ? GDALGetDataTypeSize(dstDatatype)/8 : GDALGetDataTypeSize(dstDatatype)/4; // REAL -> {32F} | CMPLX -> {32F(re),32F(im),32F(abs),32F(phase)}
    int nbBands = _dataset->GetRasterCount();

    int desiredNbOfSamples=srcImgExtent.width()*srcImgExtent.height();

    for (int i=0; i<nbBands; i++)
    {
//        GDALRasterBand * band = _dataset->GetRasterBand(i+1)->GetRasterSampleOverview(desiredNbOfSamples);
//        scaleX = band->GetXSize()*1.0/_pixelExtent.width();
//        scaleY = band->GetYSize()*1.0/_pixelExtent.height();

        GDALRasterBand * band = _dataset->GetRasterBand(i+1);
        scaleX=1.0;
        scaleY=1.0;

        CPLErr err = band->RasterIO( GF_Read,
                                     srcImgExtent.x()*scaleX,
                                     srcImgExtent.y()*scaleY,
                                     srcImgExtent.width()*scaleX,
                                     srcImgExtent.height()*scaleY,
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

}
