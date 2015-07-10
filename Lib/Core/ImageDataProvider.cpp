
// STD
#include <cmath>
#include <limits>


// GDAL
#include <gdal_priv.h>

// Qt
#include <QString>
#include <QFileInfo>
#include <qmath.h>
#include <QMutex>

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
  ImageDataProvider can be seen as a filter. Complex bands are interpreted as 4 non-complex bands (re,im,abs,phase)

  For example, input image can be of type :
  1) Single band, Non-Complex
  2) Single band, Complex
  3) Multi-bands (N), Non-Complex
  4) Multi-bands (M), Complex

  Data provider will interpret this input as :
  1) Single-band, Non-Complex
  2) 4 bands, Non-Complex = (Re,Im,Abs,Phase)
  3) Multi-bands (N), Non-Complex
  4) Multi-bands (4*M), Non-Complex = (Re_1,Im_1,Abs_1,Phase_1,...,Re_M,Im_M,Abs_M,Phase_M)


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
    _cutNoDataBRBoundary(true),
    _editable(false)
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

cv::Mat ImageDataProvider::computeMask(const cv::Mat &data, float noDataValue)
{
    cv::Mat out;
    cv::Mat mask = data != noDataValue;
    std::vector<cv::Mat> iChannels(data.channels());
    cv::split(mask, &iChannels[0]);
    out = iChannels[0];
    for (int i=1;i<iChannels.size();i++)
    {
        cv::bitwise_and(out, iChannels[i], out);
    }
    return out;

}

//******************************************************************************
/*!
 * \brief ImageDataProvider::getPixelValue
 * \param pixelCoords
 * \param isComplex in/out parameter which is setup within the method to indicate data type
 * \return QVector<double> of pixel values : (band1 value, band2 value, band3 value...) for non-complex imagery
 * and (band1 Re value, band1 Im value, band1 Abs value, band1 Phase value, band2 Re value, ... ) for complex imagery
 */
QVector<double> ImageDataProvider::getPixelValue(const QPoint &pixelCoords, bool *isComplex) const
{
    QVector<double> out;

    if (!_pixelExtent.contains(pixelCoords))
        return out;

    if (isComplex)
    {
        *isComplex = _inputIsComplex;
    }

    cv::Mat m = getImageData(QRect(pixelCoords,QSize(1,1)),1,1);

    float * data = reinterpret_cast<float*>(m.data);
    for (int i=0; i<m.channels();i++)
    {
        out << data[i];
    }

    return out;
}

//******************************************************************************
//******************************************************************************

GDALDataProvider::GDALDataProvider(QObject *parent) :
    ImageDataProvider(parent),
    _dataset(0),
    _mutex(new QMutex())
{
}

//******************************************************************************

GDALDataProvider::~GDALDataProvider()
{
    if (_dataset)
        GDALClose(_dataset);

    delete _mutex;
}

//******************************************************************************

bool GDALDataProvider::setup(const QString &filepath)
{
    if (_dataset)
        GDALClose(_dataset);

    _dataset = static_cast<GDALDataset *>(GDALOpen(filepath.toStdString().c_str(), GA_ReadOnly));
    if (!_dataset)
    {
        SD_TRACE( "GDALDataProvider::setup : Failed to open input file" )
        return false;
    }
    _filePath=filepath;

    char ** papszFileList = _dataset->GetFileList();
    if( CSLCount(papszFileList) > 0 )
    {
        _location = QString(papszFileList[0]);
    }
    CSLDestroy( papszFileList );



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

    // Define rectangle in the output matrix where the data will be written:
    cv::Rect r(qFloor(scaleX*(srcRequestedExtent.x() - srcExtent.x())),
               qFloor(scaleY*(srcRequestedExtent.y() - srcExtent.y())),
               reqScaledW,
               reqScaledH);

    // Output matrix is encoded as Float32 for each band (Real Imagery) or Float32 for (re,im,abs,phase) and for each band (Complex Imagery)
    // Matrix is initialized with noData value : -FLT_MAX + 1.0
//    int depth = 4; // REAL -> {32F} | CMPLX -> {32F(re),32F(im),32F(abs),32F(phase)}

//    out = cv::Mat(dstPixelExtent.height(),
//                  dstPixelExtent.width(),
//                  convertTypeToOpenCV(4, false, _nbBands),
//                  cv::Scalar::all(NoDataValue));

    out = cv::Mat(dstPixelExtent.height(),
                  dstPixelExtent.width(),
                  convertTypeToOpenCV(4, false, _nbBands));
    out.setTo(NoDataValue);


    cv::Mat b = out(r);
//    cv::Mat mask(b.cols, b.rows, CV_8U, cv::Scalar(0));

    // Read data:
    GDALDataType dstDatatype= (!_inputIsComplex) ? GDT_Float32 : GDT_CFloat32;
    int depth = (!_inputIsComplex) ? GDALGetDataTypeSize(dstDatatype)/8 : GDALGetDataTypeSize(dstDatatype)/4; // REAL -> {32F} | CMPLX -> {32F(re),32F(im),32F(abs),32F(phase)}
    int nbBands = _dataset->GetRasterCount(); // nbBands

//    int desiredNbOfSamples=srcImgExtent.width()*srcImgExtent.height();

    // GDAL dataset access should be limited to only one thread per IO
    // Otherwise : GeoTiff sends : ERROR 1: TIFFReadEncodedTile() failed | ERROR 1: IReadBlock failed at X offset 0, Y offset 8 |  ERROR 1: GetBlockRef failed at X block offset 0, Y block offset 8
    _mutex->lock();

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

        if (_inputIsComplex)
        {
            // Compute Abs,Phase and write into the buffer
            double re, im, abs, phase;
            for (int p=0;p<r.height;p++)
            {
                uchar * srcPtr = b.data+i*depth + p*nbBands*depth*dstPixelExtent.width();
                for (int q=0;q<r.width;q++)
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

        // get mask band :
        GDALRasterBand * maskband = band->GetMaskBand();
        // CAN NOT USE maskband->GetMaskFlags() & GMF_ALL_VALID
        // maskband->GetMaskFlags() is always GMF_ALL_VALID
        if (maskband)
        {
            cv::Mat m(r.height, r.width, CV_8U, cv::Scalar(0));
            CPLErr err = maskband->RasterIO( GF_Read,
                                             srcRequestedExtent.x(),
                                             srcRequestedExtent.y(),
                                             srcRequestedExtent.width(),
                                             srcRequestedExtent.height(),
                                             m.data, // data buffer offset : CMPLX -> {32F(re),32F(im),32F(abs),32F(phase)}
                                             r.width, // data buffer size (width)
                                             r.height,// data buffer size (height)
                                             GDT_Byte, // pixel type : REAL -> 32F | CMPLX -> 64F
                                             1, // pixel size : REAL -> nbBands * 4 {32F} | CMPLX -> nbBands * 8 = {32F(re),32F(im),32F(abs),32F(phase)} * nbBands
                                             r.width); // buffer line length
            // r.width replaces dstPixelExtent.width() for the buffer line lenght because dstPixelExtent.width()
            // can be larger than r.width but matrix m is of size (r.height, r.width)


            if (err != CE_None)
            {
                SD_TRACE( "Failed to read mask data" );
                return cv::Mat();
            }
            m /= 255;

            // check if whole matrix is filled <-> no nodata values
            double sum = cv::sum(m).val[0];
            if (sum < r.height * r.width)
            {
                // apply mask :
                uchar * mskPtr = m.data;
                int v;
                for (int p=0;p<r.height;p++)
                {
                    uchar * srcPtr = b.data+i*depth + p*nbBands*depth*dstPixelExtent.width();
                    for (int q=0;q<r.width;q++)
                    {
                        v = mskPtr[0];
                        if (v < 1)
                        {
                            reinterpret_cast<float*>(srcPtr)[0] = NoDataValue;
                        }
                        srcPtr+=depth*nbBands;
                        mskPtr++;
                    }
                }
            }
        }
    }

    _mutex->unlock();

    return out;
}

//******************************************************************************

QString GDALDataProvider::fetchProjectionRef() const
{
    QString res = _dataset->GetProjectionRef();
    return res.isEmpty() ? "Unknown" : res;
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
