#ifndef LAYERUTILS_H
#define LAYERUTILS_H

// Qt
#include <QPolygonF>
#include <QVariant>
#include <QPair>
#include <QList>

// GDAL
#include <gdal_priv.h>
#include <ogrsf_frmts.h>

// OpenCV
#include <opencv2/core/core.hpp>

// Project
#include "Global.h"
#include "LibExport.h"


namespace Core
{

class ProgressReporter : public QObject
{
    Q_OBJECT
public:
    ProgressReporter(QObject * parent = 0):
        QObject(parent),
        startValue(0),
        endValue(100)
    { }

    int startValue;
    int endValue;

signals:
    void progressValueChanged(int value);


};


//******************************************************************************
// Test methods
//******************************************************************************
/*!
 * \brief displayMat method to visualize a cv::Mat
 * \param inputImage
 * \param showMinMax
 * \param windowName
 * \param waitKey
 * \param rescale
 */
cv::Mat GIV_DLL_EXPORT displayMat(const cv::Mat & inputImage, bool showMinMax=false, const QString &windowName=QString(), bool waitKey=true, bool rescale=true);

bool GIV_DLL_EXPORT isEqual(const cv::Mat & src, const cv::Mat & dst, double tol = 1e-5);

void GIV_DLL_EXPORT printMat(const cv::Mat & inputImage, const QString &windowName=QString());

//******************************************************************************
// Geo Computation methods
//******************************************************************************
/*!
 * \brief computeGeoExtent method
 * \param dataset, pixelExtent
 * \return Geo Extent in LatLong computed for the pixelExtent using image projection info
 */
QPolygonF GIV_DLL_EXPORT computeGeoExtent(GDALDataset *dataset, const QRect & pixelExtent = QRect());

/*!
 * \brief computeGeoTransform method
 * \param dataset, geoExtent, pixelExtent
 * \return geo transform vector computed for the geoExtent using image projection info
 */
QVector<double> GIV_DLL_EXPORT computeGeoTransform(const QPolygonF &geoExtent, const QRect &pixelExtent);

/*!
 * \brief createOverviewes method to create an overview .ovr file to optimize data extraction
 * \param dataset
 * \return true if successful, false otherwise
 */
bool GIV_DLL_EXPORT createOverviews(GDALDataset * dataset, ProgressReporter *reporter=0);


/*!
 * \brief isSubsetFile method to check whether imagery contains subsets
 * \param dataset
 * \return true if file contains subsets
 */
bool GIV_DLL_EXPORT isSubsetFile(const QString & filename, QStringList &subsetNames, QStringList &subsetDescriptions);

/*!
 \brief getProjectionStrFromGeoCS method to create projection string from geographic coordinate system name
 */
QString GIV_DLL_EXPORT getProjectionStrFromGeoCS(const QString & gcs = "WGS84");

//QString GIV_DLL_EXPORT getProjectionStrFromEPSG(int epsgCode = 4326);

/*!
 \brief compareProjections method to compare projection strings
 */
bool GIV_DLL_EXPORT compareProjections(const QString & prStr1, const QString & prStr2);

/*!
 \brief isGeoProjection method to check if projection is geographic
 */
bool GIV_DLL_EXPORT isGeoProjection(const QString & prStr);

//******************************************************************************
// Image Computation methods
//******************************************************************************
/*!
 * \brief computeHistogram method to compute normalized histogram of an image layer which is used in rendering phase
 * \param
 * \return true if successful
 */
//bool computeNormalizedHistogram(ImageLayer * layer, int histSize=1000, bool isRough=true, ProgressReporter *reporter=0);
bool GIV_DLL_EXPORT computeNormalizedHistogram(const cv::Mat & data,
                                QVector<double> & minValues, QVector<double> & maxValues,
                                QVector< QVector<double> > & bandHistograms,
                                int histSize=1000, ProgressReporter *reporter=0);

/*!
 * \brief computeQuantileMinMaxValues method to compute min/max values using the histogram and quantile values (e.g. lowerCut=2.5, upperCut=97.5)
 * \param layer
 * \param lowerCut
 * \param upperCut
 * \param qMinValues - output quantile min values
 * \param qMaxValues - output quantile max values
 */
bool GIV_DLL_EXPORT computeQuantileMinMaxValues(const QVector<double> & minValues, const QVector<double> & maxValues,
                                 const QVector< QVector<double> > & bandHistograms,
                                 double lowerCut, double upperCut, QVector<double> *qMinValues, QVector<double> *qMaxValues);

bool GIV_DLL_EXPORT computeQuantileMinMaxValue(const QVector<double> & bandHistogram, double lowerCut, double upperCut,
                                double minValue, double maxValue, double *qMinValue, double *qMaxValue);

bool GIV_DLL_EXPORT computeQuantileMinMaxValue(const QVector<double> & bandHistogram, double lowerCut, double upperCut,
                                int *qMinIndex, int *qMaxIndex);


/*!
  \brief computeLocalMinMax method to compute local minima and maxima on the data (e.g. histogram) in local windows
  Input data is slightly smoothed in the procedure
 */
void GIV_DLL_EXPORT computeLocalMinMax(const QVector<double> & data,
                                       QVector<int> * minLocs, QVector<int> * maxLocs,
                                       QVector<double> * minVals=0, QVector<double> * maxVals=0,
                                       int windowSize=-1, int blurKernelSize=5);

//******************************************************************************
// File Read/Write methods
//******************************************************************************

bool GIV_DLL_EXPORT writeToFile(const QString & outputFilename, const cv::Mat & image,
                                const QString & projectionStr = QString(), const QVector<double> & geoTransform = QVector<double>(),
                                double nodatavalue = -123456789.0,
                                const QList< QPair<QString,QString> > & metadata = QList< QPair<QString,QString> >());

//******************************************************************************
// Type conversion methods
//******************************************************************************
/*!
    Method to convert opencv data depth to gdal type
 */
inline GDALDataType convertDataTypeOpenCVToGDAL(int ocvDepth, int nbBands)
{
    if (nbBands != 2)
    {
        if (ocvDepth == CV_8U)
        {
            return GDT_Byte;
        }
        else if (ocvDepth == CV_8S)
        {
            return GDT_Byte;
        }
        else if (ocvDepth == CV_16U)
        {
            return GDT_UInt16;
        }
        else if (ocvDepth == CV_16S)
        {
            return GDT_Int16;
        }
        else if (ocvDepth == CV_32F)
        {
            return GDT_Float32;
        }
        else if (ocvDepth == CV_32S)
        {
            return GDT_Int32;
        }
        else if (ocvDepth == CV_64F)
        {
            return GDT_Float64;
        }
    }
    else
    {
        if (ocvDepth == CV_16S)
        {
            return GDT_CInt16;
        }
        else if (ocvDepth == CV_32F)
        {
            return GDT_CFloat32;
        }
        else if (ocvDepth == CV_64F)
        {
            return GDT_CFloat64;
        }
    }
    return GDT_Unknown;
}

//*************************************************************************

/*!
    Method to convert {depth, isComplex, nbBands} to opencv data type
 */
inline int convertTypeToOpenCV(int depthInBytes, bool isComplex, int nbBands )
{
    if (depthInBytes == 1 && !isComplex)
    {
        return CV_8UC(nbBands);
    }
    else if (depthInBytes == 2 && !isComplex)
    {
        return CV_16UC(nbBands);
    }
    else if (depthInBytes == 2 && isComplex)
    {
        return CV_16SC2;
    }
    else if (depthInBytes == 4 && !isComplex)
    {
        return CV_32FC(nbBands);
    }
    else if (depthInBytes == 4 && isComplex)
    {
        return CV_32FC2;
    }
    else if (depthInBytes == 8 && !isComplex)
    {
        return CV_64FC(nbBands);
    }
    else if (depthInBytes == 8 && isComplex)
    {
        return CV_64FC2;
    }
    return -1;
}

//*************************************************************************

/*!
    Method to convert gdal type to opencv data type
 */
inline int convertDataTypeGDALToOpenCV( GDALDataType type, int nbBands=1 )
{
    if (type == GDT_Byte)
    {
        return CV_8UC(nbBands);
    }
    else if (type == GDT_Int16)
    {
        return CV_16SC(nbBands);
    }
    else if (type == GDT_UInt16)
    {
        return CV_16UC(nbBands);
    }
    else if (type == GDT_Int32)
    {
        return CV_32SC(nbBands);
    }
    else if (type == GDT_UInt32)
    {
        return CV_32FC(nbBands);
    }
    else if (type == GDT_Float32)
    {
        return CV_32FC(nbBands);
    }
    else if (type == GDT_Float64)
    {
        return CV_64FC(nbBands);
    }
    else if (type == GDT_CInt16)
    {
        return CV_16SC2;
    }
    else if (type == GDT_CInt32)
    {
        return CV_32FC2;
    }
    else if (type == GDT_CFloat32)
    {
        return CV_32FC2;
    }
    else if (type == GDT_CFloat64)
    {
        return CV_64FC2;
    }
    return -1;
}

//*************************************************************************

/*!
    Method to convert QVariant type into OGR type
 */
inline OGRFieldType getOGRFieldTypeFromQVariant(const QVariant & v)
{
    QVariant::Type type = v.type();

    if (type == QVariant::String)
    {
        return OFTString;
    }
    else if (type == QVariant::Double)
    {
        return OFTReal;
    }
    else if (type == QMetaType::Float)
    {
        return OFTReal;
    }
    else if (type == QVariant::Int)
    {
        return OFTInteger;
    }

    SD_TRACE( "Failed to recognize QVariant object type. Default type (OFTString) is returned" )
    return OFTString;
}

//******************************************************************************

}

int progressCallback( double dfComplete, const char * pszMessage, void * pProgressArg );

#endif // LAYERUTILS_H
