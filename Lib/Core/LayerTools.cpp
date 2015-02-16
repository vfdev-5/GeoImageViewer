
// GDAL
#include <gdal_alg.h>

// Qt
#include <qmath.h>

// OpenCV
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Project
#include "LayerTools.h"
#include "ImageLayer.h"

namespace Core
{

//******************************************************************************

QPolygonF computeGeoExtent(GDALDataset * inputDataset)
{
    QPolygonF gPts;
    QVector<QPoint> pts; // topLeft, topRight, bottomRight, bottomLeft
    int width = inputDataset->GetRasterXSize();
    int height = inputDataset->GetRasterYSize();
    pts << QPoint(0, 0) << QPoint(width, 0) << QPoint(width, height) << QPoint(0, height);

    // Compute Spatial extent as 4 corners transformation into WGS84:
    void * transformArg = 0;
    OGRSpatialReference * dstSRS = static_cast<OGRSpatialReference*>( OSRNewSpatialReference( 0 ) );
    dstSRS->SetWellKnownGeogCS( "WGS84" );
    char *dstSRSWkt = 0;
    dstSRS->exportToWkt(&dstSRSWkt);
    char ** toOpions = CSLSetNameValue( 0, "DST_SRS", dstSRSWkt );
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
                geoX = ( geoX > 180 ) ? geoX - 360 : geoX;
                geoX = ( geoX < -180 ) ? geoX + 360 : geoX;
                gPts << QPointF(geoX, geoY); // Points are Long/Lat
            }
        }
        GDALDestroyTransformer(transformArg);
    }
    dstSRS->Release();
    return gPts;
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
        windowNameS = "Input_Image";
    else
        windowNameS = windowName;

    cv::Mat inputImage;
    if (inputImage0.type() != CV_32F && inputImage0.type() != CV_64F)
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
            SD_TRACE( "displayMat : " + windowNameS + ", Image size is rescaled" );
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
        SD_TRACE( QString( "Image \'" + windowNameS + "\' has size : %1, %2" ).arg( inputImage0.cols ).arg( inputImage0.rows ) );
    }

    // compute min/max values & render:
    std::vector<double> min(nbBands), max(nbBands);
    std::vector<double> nmin(nbBands), nmax(nbBands);
    std::vector<bool> minMaxComputed;
    minMaxComputed.resize(nbBands, false);
    for (int i=0; i < mapping.size(); i ++)
    {
       int index = mapping.value(i+1) - 1;
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
       if (showMinMax)
       {
           SD_TRACE( QString( "Image " + windowNameS + ", min/max : %1, %2" ).arg( min[index] ).arg( max[index] ) );
           SD_TRACE( QString( "Image " + windowNameS + ", min/max using mean/std : %1, %2").arg( nmin[index] ).arg(  nmax[index] ) );
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

#define REPORT() \
if (reporter)   \
{ \
    taskProgressCount++;    \
    double r = taskProgressCount*1.0 / taskProgressSize; \
    reporter->progressValueChanged( \
    r*(reporter->endValue - reporter->startValue) + reporter->startValue); \
} \

bool computeNormalizedHistogram(ImageLayer *layer, int histSize, bool isRough, ProgressReporter *reporter)
{
    // Get full image into a matrix of size 512x(512*ratio)
    cv::Mat data = layer->getImageData(QRect(), (isRough) ? 512 : 0);
    if (data.empty())
        return false;


    // Compute histogram:
    int nbBands = data.channels();
    std::vector<cv::Mat> ic(nbBands);
    cv::split(data, &ic[0]);

    QVector<double> minValues(nbBands);
    QVector<double> maxValues(nbBands);

    int taskProgressSize = nbBands * 3;
    int taskProgressCount = 0;

    for (int i=0; i<nbBands;i++)
    {
        cv::Mat band = ic[i], band32F;
        band.convertTo(band32F, CV_32F);

        cv::Mat bandHistogram;
        // 1) compute range of the image : real min/max
        double mm, MM;
        cv::minMaxLoc(band32F, &mm, &MM);
        minValues[i] = mm;
        maxValues[i] = MM;


        REPORT();

        // 2) compute histogram:
        int histSizeArray[] = {histSize};
        int channels[] = {0};
        float histRangeArray[] = {(float) mm, (float) MM};
        const float * ranges[] = { histRangeArray };
        cv::calcHist( &band32F, 1, channels, cv::Mat(), bandHistogram, 1, histSizeArray, ranges, true, false ); // bool uniform=true, bool accumulate=false

        REPORT();

        cv::minMaxLoc(bandHistogram, &mm, &MM);
        QVector<double> bandHistogramVector(histSize);
        for (int k=0; k<bandHistogram.rows;k++)
        {
            bandHistogramVector[k] = (bandHistogram.at<float>(k,0) - mm)/(MM - mm);
        }
        layer->setBandHistogram(i, bandHistogramVector);


        REPORT();

    }

    layer->setMinValues(minValues);
    layer->setMaxValues(maxValues);

    return true;
}

//******************************************************************************

void computeQuantileMinMaxValue(const QVector<double> & bandHistogram, double lowerCut, double upperCut,
                                double minValue, double maxValue, double *qMinValue, double *qMaxValue);

void computeQuantileMinMaxValues(const ImageLayer * layer, double lowerCut, double upperCut, QVector<double> * qMinValues, QVector<double> * qMaxValues)
{

    const QVector<QVector<double> > & bandHistograms = layer->getBandHistograms();
    const QVector<double> & minValues = layer->getMinValues();
    const QVector<double> & maxValues = layer->getMaxValues();
    qMinValues->clear();
    qMaxValues->clear();
    double qMinValue(0), qMaxValue(0);
    for (int i=0;i<bandHistograms.size();i++)
    {
        computeQuantileMinMaxValue(bandHistograms[i], lowerCut, upperCut,
                                   minValues[i], maxValues[i], &qMinValue, &qMaxValue);
        qMinValues->append(qMinValue);
        qMaxValues->append(qMaxValue);
//        SD_TRACE(QString("Quantile min/max : %1, %2").arg(qMinValue).arg(qMaxValue));
    }
}

void computeQuantileMinMaxValue(const QVector<double> & bandHistogram, double lowerCut, double upperCut,
                                double minValue, double maxValue, double * qMinValue, double * qMaxValue)
{

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
    int minIndx, maxIndx;
    for (int i=0; i<size; i++)
    {
        v1 += srcPtr[i];
        v2 += srcPtr[size-1-i];
        r1=100.0*v1/sum;
        r2=100.0*v2/sum;
        if (r1 >= lowerCut && !lowerCutFound)
        {
            lowerCutFound=true;
            minIndx = i;
        }


        if (r2 >= 100-upperCut && !upperCutFound)
        {
            upperCutFound=true;
            maxIndx = size-1-i;
        }

        if (lowerCutFound && upperCutFound)
            break;
    }

    // compute values: xvalue(index) = xmin + (xmax - xmin)/size * index
    double step = (maxValue - minValue)*1.0/size;
    *qMinValue = minValue + step*minIndx;
    *qMaxValue = minValue + step*maxIndx;

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

