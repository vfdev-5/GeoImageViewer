#ifndef LAYERTOOLS_H
#define LAYERTOOLS_H

// Qt
#include <QPolygonF>
#include <QVariant>

// GDAL
#include <gdal_priv.h>
#include <ogrsf_frmts.h>

// OpenCV
#include <opencv2/core/core.hpp>

// Project
#include "Global.h"


namespace Core
{

class ImageLayer;

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
// Test visualization methods
//******************************************************************************
/*!
 * \brief displayMat method to visualize a cv::Mat
 * \param inputImage
 * \param showMinMax
 * \param windowName
 * \param waitKey
 * \param rescale
 */
cv::Mat displayMat(const cv::Mat & inputImage, bool showMinMax=false, const QString &windowName=QString(), bool waitKey=true, bool rescale=true);

//******************************************************************************
// Geo Computation methods
//******************************************************************************
/*!
 * \brief computeGeoExtent method
 * \param dataset
 * \return Geo Extent in LatLong computed from image corners using image projection info
 */
QPolygonF computeGeoExtent(GDALDataset * dataset);

/*!
 * \brief createOverviewes method to create an overview .ovr file to optimize data extraction
 * \param dataset
 * \return true if successful, false otherwise
 */
bool createOverviews(GDALDataset * dataset, ProgressReporter *reporter=0);


/*!
 * \brief isSubsetFile method to check whether imagery contains subsets
 * \param dataset
 * \return true if file contains subsets
 */
bool isSubsetFile(const QString & filename, QStringList &subsetNames, QStringList &subsetDescriptions);

//******************************************************************************
// Image Computation methods
//******************************************************************************
/*!
 * \brief computeHistogram method to compute normalized histogram of an image layer which is used in rendering phase
 * \param ImageLayer
 * \return true if successful
 */
bool computeNormalizedHistogram(ImageLayer * layer, int histSize=1000, bool isRough=true, ProgressReporter *reporter=0);

/*!
 * \brief computeQuantileMinMaxValues method to compute min/max values using the histogram and quantile values (e.g. lowerCut=2.5, upperCut=97.5)
 * \param layer
 * \param lowerCut
 * \param upperCut
 * \param qMinValues - output quantile min values
 * \param qMaxValues - output quantile max values
 */
void computeQuantileMinMaxValues(const ImageLayer * layer, double lowerCut, double upperCut, QVector<double> *qMinValues, QVector<double> *qMaxValues);

//******************************************************************************
// Type conversion methods
//******************************************************************************
/*!
    Method to convert opencv data type to gdal type
 */
inline GDALDataType convertDataTypeOpenCVToGDAL(int ocvType)
{
    if (ocvType == CV_8U)
    {
        return GDT_Byte;
    }
    else if (ocvType == CV_8S)
    {
        return GDT_Byte;
    }
    else if (ocvType == CV_16U)
    {
        return GDT_UInt16;
    }
    else if (ocvType == CV_16S)
    {
        return GDT_Int16;
    }
    else if (ocvType == CV_32F)
    {
        return GDT_Float32;
    }
    else if (ocvType == CV_32S)
    {
        return GDT_Int32;
    }
    else if (ocvType == CV_64F)
    {
        return GDT_Float64;
    }
    else if (ocvType == CV_16SC2)
    {
        return GDT_CInt16;
    }
    else if (ocvType == CV_32FC2)
    {
        return GDT_CFloat32;
    }
    else if (ocvType == CV_64FC2)
    {
        return GDT_CFloat64;
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

#endif // LAYERTOOLS_H
