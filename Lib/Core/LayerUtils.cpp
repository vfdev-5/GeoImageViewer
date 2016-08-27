
// GDAL
#include <gdal_alg.h>

// Qt
#include <qmath.h>
#include <QString>
#include <QFileInfo>

// OpenCV
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Project
#include "LayerUtils.h"

namespace Core
{

//******************************************************************************

//QPolygonF computeGeoExtent(GDALDataset * inputDataset, const QRect & pixelExtent)
QPolygonF computeGeoExtent(GDALDataset * inputDataset, const QVector<QPoint> & pts0)
{
    QPolygonF gPts;
    QVector<QPoint> pts; // topLeft, topRight, bottomRight, bottomLeft

    if (pts0.isEmpty())
    {
        int width = inputDataset->GetRasterXSize();
        int height = inputDataset->GetRasterYSize();
        pts << QPoint(0, 0) << QPoint(width-1, 0) << QPoint(width-1, height-1) << QPoint(0, height-1);
    }
    else
    {
//        pts << pixelExtent.topLeft() << pixelExtent.topRight()
//            << pixelExtent.bottomRight() << pixelExtent.bottomLeft();
        pts=pts0;
    }

    // Compute Spatial extent as 4 corners transformation into inputDataset projection:
    void * transformArg = 0;
//    OGRSpatialReference * dstSRS = static_cast<OGRSpatialReference*>( OSRNewSpatialReference( 0 ) );
//    dstSRS->SetWellKnownGeogCS( "WGS84" );
//    char *dstSRSWkt = 0;
//    dstSRS->exportToWkt(&dstSRSWkt);
//    char ** toOpions = CSLSetNameValue( 0, "DST_SRS", dstSRSWkt );
    char ** toOpions = CSLSetNameValue( 0, "DST_SRS", inputDataset->GetProjectionRef() );
    GDALTransformerFunc Transformer = GDALGenImgProjTransform;

    transformArg = GDALCreateGenImgProjTransformer2(inputDataset, 0, toOpions);
    if (transformArg != 0)
    {
        foreach ( QPoint pt, pts)
        {
            double geoX = pt.x();
            double geoY = pt.y();
            double z = 0.0;
            int isSuccess = false;
            Transformer( transformArg, false, 1, &geoX, &geoY, &z, &isSuccess );
            if ( isSuccess )
            {
//                geoX = ( geoX > 180 ) ? geoX - 360 : geoX;
//                geoX = ( geoX < -180 ) ? geoX + 360 : geoX;
                gPts << QPointF(geoX, geoY); // Points are Long/Lat
            }
        }
        GDALDestroyTransformer(transformArg);
    }
//    dstSRS->Release();
//    OGRFree(dstSRSWkt);
    return gPts;
}

//*************************************************************************

QVector<double> computeGeoTransform(const QPolygonF & geoExtent, const QRect & pixelExtent)
{
    if (geoExtent.isEmpty() || geoExtent.size() != 4
            || pixelExtent.isEmpty())
        return QVector<double>();
    // Output geoTransform :
    // X = gt[0] + px*gt[1] + py*gt[2]
    // Y = gt[3] + px*gt[4] + py*gt[5]
    QVector<double> out(6);
    out[0] = geoExtent[0].x();
    out[3] = geoExtent[0].y();
    out[1] = ( geoExtent[1].x() - out[0] ) * 1.0 / (pixelExtent.width() - 1);
    out[4] = ( geoExtent[1].y() - out[3] ) * 1.0 / (pixelExtent.width() - 1);
    out[2] = ( geoExtent[3].x() - out[0] ) * 1.0 / (pixelExtent.height() - 1);
    out[5] = ( geoExtent[3].y() - out[3] ) * 1.0 / (pixelExtent.height() - 1);

    return out;

}

//*************************************************************************

bool createOverviews(GDALDataset *dataset, ProgressReporter *reporter)
{
    if (dataset->GetRasterBand(1)->GetOverviewCount() > 0)
    {
        SD_TRACE("Overview file is found");
        return true;
    }

    // Compute number of levels
    int minTileSize = 512;
    int imageMaxDim = qMin(dataset->GetRasterXSize(), dataset->GetRasterYSize());
    int numberOfPyrLevels = qFloor(qLn(imageMaxDim*1.0/minTileSize)/qLn(2.0));
    if (numberOfPyrLevels < 1)
    {
        return true;
    }
    QVector<int> decimationFactors(numberOfPyrLevels);

    decimationFactors[0]=2;
    for (int i=1; i<decimationFactors.size(); i++)
    {
        decimationFactors[i]=2*decimationFactors[i-1];
    }

    if ( dataset->BuildOverviews("NEAREST", numberOfPyrLevels, decimationFactors.data(), 0, 0, progressCallback, reporter) != CE_None)
    {
        SD_TRACE(QObject::tr("GDAL failed to build overviews"));
        return false;
    }

    if (dataset->GetRasterBand(1)->GetOverviewCount() == 0)
    {
        SD_TRACE(QObject::tr("Failed to find the output file"));
        return false;
    }

    return true;
}

//*************************************************************************

bool isSubsetFile(const QString & filepath, QStringList & subsetNames, QStringList & subsetDescriptions)
{
    GDALDataset* dataset = static_cast<GDALDataset *>(GDALOpen( filepath.toStdString().c_str(), GA_ReadOnly ));
    if( !dataset )
    {
        SD_ERR("GDAL failed to open the file");
        return false;
    }

    char **papszSubdatasets = GDALGetMetadata( dataset, "SUBDATASETS" );
    int nSubdatasets = CSLCount( papszSubdatasets );
    if (nSubdatasets == 0)
    {
        GDALClose( dataset );
        return false;
    }
    for (int i=0; i<nSubdatasets; i+=2)
    {
        QString subsetName=papszSubdatasets[i];
        subsetName = subsetName.section("=",1);
        QString subsetDescription=papszSubdatasets[i+1];
        subsetDescription = subsetDescription.section("=", 1);
        // test if subset is not a QuickLook :
        if (!subsetDescription.contains("QLK", Qt::CaseInsensitive))
        {
            subsetNames << subsetName;
            subsetDescriptions << subsetDescription;
        }
    }

    GDALClose( dataset );
    return true;
}

//*************************************************************************

cv::Mat displayMat(const cv::Mat & inputImage0, bool showMinMax, const QString & windowName, bool waitKey, bool rescale)
{
    QString windowNameS;
    if (windowName.isEmpty())
        windowNameS = "'Input_Image'";
    else
        windowNameS = windowName;

    int depth = inputImage0.elemSize1();

    cv::Mat inputImage;
    int inputDepth = inputImage0.depth();
    if (inputDepth != CV_32F && inputDepth != CV_64F)
    {
        inputImage0.convertTo(inputImage, CV_32F);
    }
    else
        inputImage = inputImage0;

    int nbBands = inputImage0.channels();
    QMap<int,int> mapping;
    if (nbBands == 1)
    {
        mapping.insert(1, 1);
        mapping.insert(2, 1);
        mapping.insert(3, 1);
    }
    else if (nbBands == 2)
    {
        mapping.insert(1, 1);
        mapping.insert(2, 1);
        mapping.insert(3, 1);

        std::vector<cv::Mat> ic(nbBands);
        cv::split(inputImage, &ic[0]);
        cv::magnitude(ic[0], ic[1], inputImage);
        nbBands=1;
    }
    else if (nbBands >= 3)
    {
        mapping.insert(1, 3);
        mapping.insert(2, 2);
        mapping.insert(3, 1);
    }



    if (rescale)
    {
        // resize image if max dimension is larger displayLimit=1000 pixels
        int displayLimit=800;
        int maxdim = inputImage.rows > inputImage.cols ? inputImage.rows : inputImage.cols;
        if (maxdim > displayLimit)
        {
            SD_TRACE( "displayMat : '" + windowNameS + "', Image size is rescaled" );
            double f = maxdim * 1.0 / displayLimit;
            cv::resize(inputImage, inputImage, cv::Size(0,0), 1.0/f, 1.0/f);
        }
    }

    cv::Mat outputImage8U;
    std::vector<cv::Mat> iChannels(nbBands);
    std::vector<cv::Mat> oChannels(mapping.size());
    cv::split(inputImage, &iChannels[0]);

    // show image size :
    if (showMinMax)
    {
        SD_TRACE( QString( "Image \'" + windowNameS + "\' has size : %1, %2 and nbBands = %3. Depth (bytes) = %4" )
                  .arg( inputImage0.cols )
                  .arg( inputImage0.rows )
                  .arg( inputImage0.channels())
                  .arg( depth ));
    }

    // compute min/max values & render:
    std::vector<double> min(nbBands), max(nbBands);
    std::vector<double> nmin(nbBands), nmax(nbBands);
    std::vector<bool> minMaxComputed;
    minMaxComputed.resize(nbBands, false);
    for (int i=0; i < mapping.size(); i ++)
    {
       int index = mapping.value(i+1) - 1;
       if (inputDepth != CV_8U)
       {
           if (!minMaxComputed[index])
           {
               cv::minMaxLoc(iChannels[index], &min[index], &max[index]);
               cv::Scalar mean, std;
               cv::meanStdDev(iChannels[index], mean, std);
               nmin[index] = mean.val[0] - 3.0*std.val[0];
               nmin[index] = (nmin[index] < min[index]) ? min[index] : nmin[index];
               nmax[index] = mean.val[0] + 3.0*std.val[0];
               nmax[index] = (nmax[index] > max[index]) ? max[index] : nmax[index];
           }
       }
       else
       {
           nmin[index] = min[index] = 0;
           nmax[index] = max[index] = 255;
       }

       if (showMinMax)
       {
           SD_TRACE( QString( "Image '" + windowNameS + "', min/max : %1, %2" ).arg( min[index] ).arg( max[index] ) );
           SD_TRACE( QString( "Image '" + windowNameS + "', min/max using mean/std : %1, %2").arg( nmin[index] ).arg(  nmax[index] ) );
       }
       double a(1.0);
       double b(0.0);

       if (nmin[index] < nmax[index])
       {
           a = 255.0 / ( nmax[index] - nmin[index] );
           b = - 255.0 * nmin[index] / ( nmax[index] - nmin[index] );
       }
       iChannels[index].convertTo(oChannels[i], CV_8U, a, b);
    }

    cv::merge(oChannels, outputImage8U);
    cv::imshow(windowNameS.toStdString().c_str(), outputImage8U);

    if (waitKey)
        cv::waitKey(0);

    return outputImage8U;

}

//******************************************************************************

bool isEqual(const cv::Mat &src, const cv::Mat &dst, double tol)
{
    if (src.type() != dst.type())
        return false;

    if (src.rows != dst.rows || src.cols != dst.cols)
        return false;

    int nbBands = src.channels();

    std::vector<cv::Mat> sChannels(nbBands), dChannels(nbBands);
    cv::split(src, &sChannels[0]);
    cv::split(dst, &dChannels[0]);

    for (int i=0; i<nbBands; i++)
    {
        cv::Mat d;
        cv::absdiff(sChannels[i], dChannels[i], d);
        cv::Scalar s = cv::sum(d);
        if (qAbs(s.val[0]) > tol)
        {
            return false;
        }
    }
    return true;
}

//******************************************************************************
/*!
    Method to compute the mask of the data ignoring pixels with noDataValue.
    \param data input matrix can be of any depth and number of channels
    \param unmask is optional output parameter to obtain no-data mask (complementary of returned mask)

    \return output mask will have the same number of channels as \param data
    mask pixel value equals 1 if there is a data different of noDataValue
    in the corresponding channel in \param data.
 */
cv::Mat computeMask(const cv::Mat &data, float noDataValue, cv::Mat * unmask)
{
    cv::Mat mask;
    cv::Mat m = data != noDataValue;
    m.convertTo(mask, data.depth(), 1.0/255.0);
    if (unmask)
    {
        cv::Mat m2 = data == noDataValue;
        m2.convertTo(*unmask, data.depth(), 1.0/255.0);
    }
    return mask;
}

//******************************************************************************

QVector<QPolygonF> vectorizeAsPolygons(const cv::Mat & inputImage, bool externalOnly, bool approx)
{
    QVector<QPolygonF> output;

    if (inputImage.type() != CV_8U)
        return output;


    std::vector<std::vector<cv::Point> > contours;
    int mode = externalOnly ? cv::RETR_EXTERNAL : cv::RETR_CCOMP;
    int method = approx ? cv::CHAIN_APPROX_NONE : cv::CHAIN_APPROX_SIMPLE;
    cv::findContours(inputImage, contours, mode, method);

    std::vector<std::vector<cv::Point> >::iterator it = contours.begin();
    for (;it != contours.end(); ++it)
    {
        const std::vector<cv::Point> & contour = *it;
        QPolygonF c;
        for (int i=0;i<contour.size();i++)
        {
            c << QPointF(contour[i].x, contour[i].y);
        }
        // close polygon:
        c << QPointF(contour[0].x, contour[0].y);
        output << c;
    }

    // - contours represents array of array of contour points
    // - hierarchy represents array of (next, prev, 1st child, parent) contour indices
    // e.g. if hierarchy[i]["parent"] = -1 -> outer contour
    // e.g. if hierarchy[i]["parent"] > 0 -> inner contour
//    std::vector<std::vector<cv::Point> > contours;
//    std::vector<cv::Vec4i> hierarchy;
//    cv::findContours(inputImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
//    if (hierarchy.size() > 0)
//    {
//        int index = 0;
//        while (index >= 0)
//        {
//            // if has parent -> skip because it has been already added
//            if (hierarchy[index][3] >= 0)
//                continue;

//            std::vector<cv::Point> contour = contours[index];
//            QPolygonF c;
//            for (int i=0;i<contour.size();i++)
//            {
//                c << QPointF(contour[i].x, contour[i].y);
//            }
//            // close polygon:
//            c << QPointF(contour[0].x, contour[0].y);
//            output << c;
//            index = hierarchy[index][0];
//        }
//    }

    return output;
}


//******************************************************************************

template<typename T>
void printPixel(const cv::Mat & singleBand, int x, int y)
{

    std::cout << singleBand.at<T>(y,x);
}

// Print matrix data:
void printMat(const cv::Mat & inputImage0, const QString &windowName, int limit)
{

    cv::Mat inputImage;
    if (inputImage0.depth() != CV_32F)
    {
        inputImage0.convertTo(inputImage, CV_32F);
    }
    else
        inputImage = inputImage0;

    QString t = windowName.isEmpty() ? "inputImage" : windowName;
    SD_TRACE(QString("------ Print matrix : ") + t);
    int w = inputImage.cols;
    int h = inputImage.rows;
    SD_TRACE("Size : " + QString::number(w) + ", " + QString::number(h));


    w = w > limit ? limit : w;
    h = h > limit ? limit : h;
    int nbBands = inputImage.channels();
    std::vector<cv::Mat> iChannels(nbBands);
    cv::split(inputImage, &iChannels[0]);

    for (int i=0; i<h; i++)
    {
        for (int j=0; j<w; j++)
        {
            std::cout << "(";
            for (int k=0; k<nbBands; k++)
            {
                printPixel<float>(iChannels[k], j, i);
                std::cout << " ";
            }
            std::cout << ")";
        }
        std::cout << std::endl;
    }
    std::cout << "------"<< std::endl;


}

//******************************************************************************

#define REPORT() \
if (reporter)   \
{ \
    taskProgressCount++;    \
    double r = taskProgressCount*1.0 / taskProgressSize; \
    reporter->progressValueChanged( \
    r*(reporter->endValue - reporter->startValue) + reporter->startValue); \
} \

bool computeNormalizedHistogram(const cv::Mat & data, const cv::Mat & noDataMask,
                                QVector<double> & minValues, QVector<double> & maxValues,
                                QVector< QVector<double> > & bandHistograms,
                                int histSize, ProgressReporter *reporter)
{

    // Compute histogram:
    int nbBands = data.channels();
    std::vector<cv::Mat> ic(nbBands);
    cv::split(data, &ic[0]);

    minValues.clear();
    minValues.resize(nbBands);
    maxValues.clear();
    maxValues.resize(nbBands);

    bandHistograms.clear();

    int taskProgressSize = nbBands * 3;
    int taskProgressCount = 0;

    for (int i=0; i<nbBands;i++)
    {
        cv::Mat band = ic[i], band32F;
        band.convertTo(band32F, CV_32F);

        cv::Mat bandHistogram;
        // 1) compute range of the image : real min/max
        double mm, MM;
        cv::minMaxLoc(band32F, &mm, &MM, 0, 0, noDataMask);
        minValues[i] = mm;
        maxValues[i] = MM;

        REPORT();

        // 2) compute histogram:
        QVector<double> bandHistogramVector;
        if (mm == MM)
        {
            bandHistogramVector.fill(mm, 1);
        }
        else
        {
            int histSizeArray[] = {histSize};
            int channels[] = {0};
            float delta = 0.001*(MM-mm);
            float histRangeArray[] = {(float) mm, (float) MM + delta};
            const float * ranges[] = { histRangeArray };
            cv::calcHist( &band32F, 1, channels, noDataMask, bandHistogram, 1, histSizeArray, ranges, true, false ); // bool uniform=true, bool accumulate=false

            REPORT();

            cv::minMaxLoc(bandHistogram, &mm, &MM);
            bandHistogramVector.fill(0.0, histSize);
            if (MM > mm)
            {
                for (int k=0; k<bandHistogram.rows;k++)
                {
                    bandHistogramVector[k] = (bandHistogram.at<float>(k,0) - mm)/(MM - mm);
                }
            }
        }
        bandHistograms << bandHistogramVector;
        REPORT();

    }

    return true;
}


//******************************************************************************
/*!
 * \brief computeQuantileMinMaxValues
 * \param minValues
 * \param maxValues
 * \param bandHistograms
 * \param lowerCut is value between 0.0 and 100.0
 * \param upperCut is value between 0.0 and 100.0 and larger than upperCut
 * \param qMinValues
 * \param qMaxValues
 */
bool computeQuantileMinMaxValues(const QVector<double> & minValues, const QVector<double> & maxValues,
                                 const QVector< QVector<double> > & bandHistograms,
                                 double lowerCut, double upperCut, QVector<double> * qMinValues, QVector<double> * qMaxValues)
{
    qMinValues->clear();
    qMaxValues->clear();
    double qMinValue(0), qMaxValue(0);
    for (int i=0;i<bandHistograms.size();i++)
    {
        if (qAbs(maxValues[i] - minValues[i]) > 1e-8)
        {
            if (!computeQuantileMinMaxValue(bandHistograms[i], lowerCut, upperCut,
                                            minValues[i], maxValues[i], &qMinValue, &qMaxValue))
            {
                return false;
            }
        }
        else
        {
            qMinValue=qMaxValue=minValues[i];
        }
        qMinValues->append(qMinValue);
        qMaxValues->append(qMaxValue);
    }
    return true;
}

bool computeQuantileMinMaxValue(const QVector<double> & bandHistogram, double lowerCut, double upperCut,
                                double minValue, double maxValue, double * qMinValue, double * qMaxValue)
{
    if (!qMinValue || !qMaxValue)
        return false;

    int minIndx, maxIndx;
    if (!computeQuantileMinMaxValue(bandHistogram, lowerCut, upperCut,
                               &minIndx, &maxIndx))
        return false;

    // compute values: xvalue(index) = xmin + (xmax - xmin)/size * index
    double step = (maxValue - minValue)*1.0/bandHistogram.size();
    *qMinValue = minValue + step*minIndx;
    *qMaxValue = minValue + step*maxIndx;
    return true;
}

bool computeQuantileMinMaxValue(const QVector<double> & bandHistogram, double lowerCut, double upperCut,
                                int * qMinIndex, int * qMaxIndex)
{

    if (!qMinIndex || !qMaxIndex)
        return false;

    if (lowerCut < 0.0 || lowerCut > 100.0)
    {
        SD_TRACE("computeQuantileMinMaxValue : lowerCut should be between 0.0 and 1.0");
        return false;
    }

    if (upperCut < 0.0 || upperCut > 100.0 || upperCut < lowerCut)
    {
        SD_TRACE("computeQuantileMinMaxValue : upperCut should be between 0.0 and 1.0 and larger than lowerCut");
        return false;
    }

    const double * srcPtr = &(bandHistogram.data())[0];
    int size = bandHistogram.size();
    double sum=0;
    for (int i=0; i<size; i++)
    {
        sum += srcPtr[i];
    }

    double v1=0.0;
    double v2=0.0;
    double r1=0.0;
    double r2=0.0;
    bool lowerCutFound=false;
    bool upperCutFound=false;
    for (int i=0; i<size; i++)
    {
        v1 += srcPtr[i];
        v2 += srcPtr[size-1-i];
        r1=100.0*v1/sum;
        r2=100.0*v2/sum;
        if (r1 >= lowerCut && !lowerCutFound)
        {
            lowerCutFound=true;
            *qMinIndex = i;
        }


        if (r2 >= 100-upperCut && !upperCutFound)
        {
            upperCutFound=true;
            *qMaxIndex = size-1-i;
        }

        if (lowerCutFound && upperCutFound)
            break;
    }
    return true;
}

//******************************************************************************

void computeLocalMinMax(const QVector<double> & data,
                        QVector<int> * minLocs, QVector<int> * maxLocs,
                        QVector<double> * minVals, QVector<double> * maxVals,
                        int windowSize, int blurKernelSize)
{

    if (!minLocs || !maxLocs)
    {
        SD_TRACE("computeLocalMinMax : minLocs, maxLocs, minVals and maxVals should be non null")
        return;
    }

    minLocs->clear();
    maxLocs->clear();
    if (minVals)
        minVals->clear();

    if (maxVals)
        maxVals->clear();


    int size = windowSize;
    if (windowSize < 0)
    {
        size = data.size() / 10;
    }
    int nbWindows = qCeil( data.size() * 1.0 / size );
    int offset = 0;
    int sz = size;
    for (int i=0; i<nbWindows; i++)
    {
        offset = i*size;
        if (offset + sz > data.size())
        {
            sz = data.size() - offset;
        }

        // get data
        QVector<double> d = data.mid(offset, sz);
        cv::Mat wdata(1,sz,CV_64F,reinterpret_cast<void*>(d.data()));
//        SD_TRACE("orginal window size : " + QString::number(wdata.cols));

        // smooth data. data remain of the same size as original
        cv::Mat fwdata;
        cv::blur(wdata,fwdata,cv::Size(blurKernelSize,1));

//        SD_TRACE("new window size : " + QString::number(fwdata.cols));

        // get local min/max and its values
        double mm, MM;
        cv::Point mmLoc, MMLoc;
        cv::minMaxLoc(fwdata, &mm, &MM, &mmLoc, &MMLoc);

        if ( qAbs(mm-MM) < 0.005)
            continue;


        if (mmLoc.x > 0 && mmLoc.x < sz - 1)
        {
            SD_TRACE( QString("Local min : %1 at %2").arg(mm).arg(offset + mmLoc.x) );
            minLocs->append(offset + mmLoc.x);
            if (minVals)
                minVals->append(mm);
        }

        if (MMLoc.x > 0 && MMLoc.x < sz - 1)
        {
            SD_TRACE( QString("Local max : %1 at %2").arg(MM).arg(offset + MMLoc.x) );
            maxLocs->append(offset + MMLoc.x);
            if (maxVals)
                maxVals->append(MM);
        }
    }

}

//*****************************************************************************

//QString getProjectionStrFromEPSG(int epsgCode)
//{
//    // !!! DOES NOT WORK WITH epsgCode=4326
//    OGRSpatialReference * dstSRS = static_cast<OGRSpatialReference*>( OSRNewSpatialReference( 0 ) );
//    char *dstSRSWkt = 0;
//    OGRErr err = dstSRS->importFromEPSG(epsgCode);
//    if (err != OGRERR_NONE)
//        return QString();
//    dstSRS->exportToWkt(&dstSRSWkt);
//    QString out = QString::fromLatin1(dstSRSWkt);
//    dstSRS->Release();
//    OGRFree(dstSRSWkt);
//    return out;
//}

//*****************************************************************************

QString getProjectionStrFromGeoCS(const QString & gcs)
{
    OGRSpatialReference * dstSRS = static_cast<OGRSpatialReference*>( OSRNewSpatialReference( 0 ) );
    char *dstSRSWkt = 0;
    dstSRS->SetWellKnownGeogCS( gcs.toStdString().c_str() );
    dstSRS->exportToWkt(&dstSRSWkt);
    QString out = QString::fromLatin1(dstSRSWkt);
    dstSRS->Release();
    OGRFree(dstSRSWkt);
    return out;
}

//*****************************************************************************

bool compareProjections(const QString & prStr1, const QString & prStr2)
{
    OGRSpatialReference * srcSRS1 = static_cast<OGRSpatialReference*>( OSRNewSpatialReference( prStr1.toStdString().c_str() ) );
    if (!srcSRS1)
        return false;
    OGRSpatialReference * srcSRS2 = static_cast<OGRSpatialReference*>( OSRNewSpatialReference( prStr2.toStdString().c_str() ) );
    if (!srcSRS2)
    {
        srcSRS1->Release();
        return false;
    }



    bool res = srcSRS1->IsSame(srcSRS2) == 1;

    srcSRS1->Release();
    srcSRS2->Release();
    return res;
}

//******************************************************************************

bool isGeoProjection(const QString &prStr)
{
    OGRSpatialReference * srcSRS = static_cast<OGRSpatialReference*>( OSRNewSpatialReference( prStr.toStdString().c_str() ) );
    if (!srcSRS)
        return false;

    bool res = srcSRS->IsGeographic() == 1;

    srcSRS->Release();
    return res;
}

//******************************************************************************

bool writeToFile(const QString &outputFilename0, const cv::Mat &image,
                 const QString & projectionStr, const QVector<double> &geoTransform,
                 double nodatavalue,
                 const QList< QPair<QString,QString> > & metadata)
{
    int count = GetGDALDriverManager()->GetDriverCount();
    if (count == 0)
    {
        SD_TRACE("writeToFile : Error : GDAL drivers are not registered");
        return false;
    }

    // initialize gtiff driver to write output :
    GDALDriver * driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (!driver)
    {
        SD_TRACE( "GDAL failed to get geotiff driver to write output image " );
        return false;
    }

    int width = image.cols;
    int height = image.rows;

    // if image has two bands -> it is interpreted as complex
    int outputNbBands = image.channels() == 2 ? 1 : image.channels();
    GDALDataType dataType = convertDataTypeOpenCVToGDAL(image.depth(), outputNbBands);

    char **papszCreateOptions = 0;
    papszCreateOptions=CSLAddString(papszCreateOptions, "COMPRESS=LZW");

    QString outputFilename = outputFilename0;
    QFileInfo fi(outputFilename);
    if (fi.suffix().compare("tif", Qt::CaseInsensitive))
    {
        outputFilename.append(".tif");
    }

    GDALDataset * outputDataset = driver->Create(outputFilename.toStdString().c_str(), width, height, outputNbBands, dataType, papszCreateOptions);
    if (!outputDataset)
    {
        SD_TRACE( QString( "GDAL failed to create output image : %1" ).arg( outputFilename ) )
        return false;
    }

    // copy geo info:
    if (!projectionStr.isEmpty())
    {
        outputDataset->SetProjection( projectionStr.toStdString().c_str() );
    }
    // set geotransform:
    if (!geoTransform.isEmpty())
    {
        outputDataset->SetGeoTransform( (double*)geoTransform.data() );
    }

    // set metadata:
    typedef QPair<QString,QString> Mdi;
    foreach(Mdi item, metadata)
    {
        outputDataset->SetMetadataItem(item.first.toStdString().c_str(),
                                       item.second.toStdString().c_str());
    }

    // copy data:
    std::vector<cv::Mat> iChannels(outputNbBands);
    cv::split(image, &iChannels[0]);
    int w = image.cols;
    int h = image.rows;
    for (int i=0; i<outputNbBands; i++)
    {
        GDALRasterBand * dstBand =  outputDataset->GetRasterBand(i+1);
        if (!dstBand)
        {
            SD_TRACE( "Failed to write data" );
            return false;
        }
        CPLErr err = dstBand->RasterIO( GF_Write, 0, 0, w, h, iChannels[i].data, w, h, dataType, 0, 0 );
        if (err != CE_None)
        {
            SD_TRACE( "Failed to write data" );
            return false;
        }
        // set nodatavalue:
        if (nodatavalue != -123456789.0)
        {
            dstBand->SetNoDataValue(nodatavalue);
        }
    }


    // close datasets:
    GDALClose(outputDataset);

    return true;

}

//******************************************************************************

int joinOvrlContours(QVector<QPolygon> &contours)
{
    int count = contours.size();
    QVector< QPolygon >::iterator it1 = contours.begin();
    QVector< QPolygon >::iterator it2;
    bool erase = false;
    for (;it1 != contours.end();)
    {
        QPolygon & p1 = *it1;
        it2 = it1+1;
        for (;it2 != contours.end();)
        {
            QPolygon & p2 = *it2;

            QPolygon ints = p1.intersected(p2);
            if (ints.isEmpty())
            {
                ++it2;
                continue;
            }
            p1 = p1.united(p2);
            contours.erase(it2);
            erase = true;
            break;
        }

        if (erase)
        {
            erase = false;
        }
        else
        {
            ++it1;
        }
    }
    return count - contours.size();
}

//******************************************************************************

}

//******************************************************************************
/*!
    Global method to report the progress from GDAL methods: build overview
*/
int progressCallback( double dfComplete, const char * pszMessage, void * pProgressArg )
{
    Core::ProgressReporter *reporter = static_cast<Core::ProgressReporter *>( pProgressArg );
    if (reporter == 0)
    {
        return false;
    }

    int value = dfComplete * (reporter->endValue - reporter->startValue) + reporter->startValue;
    reporter->progressValueChanged(value);
    return true;
}

