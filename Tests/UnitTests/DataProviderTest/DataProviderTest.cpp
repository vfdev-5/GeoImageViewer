
// Qt
#include <QDir>
#include <QFileInfo>
#include <QThreadPool>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Tests
#include "DataProviderTest.h"
#include "Core/LayerUtils.h"

namespace Tests
{

int WIDTH  = 2000;
int HEIGHT = 2000;

//int DEPTH  = CV_16UC(5);
//cv::Mat TEST_MATRIX;
//QString PROJECTION_STR;
//QVector<double> GEO_TRANSFORM;
//double NO_DATA_VALUE=(1<<16) - 1;
//QList< QPair<QString,QString> > METADATA;
//QPolygonF GEO_EXTENT;


bool compareVectors(const QVector<double> & v1, const QVector<double> & v2, double tol = 1e-8)
{
    if (v1.size() != v2.size())
        return false;

    for (int i=0;i<v1.size();i++)
    {
        if (qAbs(v1[i] - v2[i]) > tol)
            return false;
    }
    return true;
}

bool comparePolygons(const QPolygonF & v1, const QPolygonF & v2, double tol = 1e-5)
{
    if (v1.size() != v2.size())
        return false;

    for (int i=0;i<v1.size();i++)
    {
        QPointF p = v1[i] - v2[i];
        if (qSqrt(p.x()*p.x() + p.y()*p.y()) > tol)
            return false;
    }
    return true;
}

cv::Mat computeMaskND(const cv::Mat & data, float noDataValue)
{
    cv::Mat m = data != noDataValue;
    cv::Mat out;
    m.convertTo(out, data.depth(), 1.0/255.0);
    return out;
}


cv::Mat createSynthMat16UC5(int width, int height)
{
    cv::Mat out = cv::Mat(width, height, CV_16UC(5));
    out.setTo(0);
    for (int i=0; i<out.rows;i++)
    {
        for (int j=0; j<out.cols;j++)
        {
            ushort c1 = (ushort) ( 100 + 1.5*i + 3.4*j +
                                   (out.rows-1-i)*j*0.01 +
                                   i*(out.rows-1-i)*(out.cols*0.5-1-j)*0.001 +
                                   j*j*0.002);
            c1 = c1 == (1<<16) - 1 ? c1-10 : c1;

            ushort c2 = (ushort) ( 10 + 2.5*i - 1.4*j +
                                   (out.rows-1-i)*j*0.02 +
                                   i*(out.rows-1-i)*(out.cols*0.3-1-j)*0.01 +
                                   j*j*0.0012);
            c2 = c2 == (1<<16) - 1 ? c2-10 : c2;

            ushort c3 = (ushort) ( 1.5*i + 5.4*j +
                                   (out.rows-1-i)*j*0.01 +
                                   i*(out.rows-1-i)*(out.cols*0.5-1-j)*0.001 +
                                   j*j*0.002);
            c3 = c3 == (1<<16) - 1 ? c3-10 : c3;

            ushort c4 = (ushort) ( 50 + 1.5*i + 4.4*j +
                                   (out.rows*0.5-1-i)*j*0.01 +
                                   i*(out.rows-1-i)*(out.cols*0.7-1-j)*0.001 +
                                   j*j*0.01);
            c4 = c4 == (1<<16) - 1 ? c4-10 : c4;

            ushort c5 = (ushort) ( c1 + 2*c2 - c3 - c4);
            c5 = c5 == (1<<16) - 1 ? c5-10 : c5;

            cv::Vec<ushort, 5> pixel = cv::Vec<ushort, 5>(c1,c2,c3,c4,c5);
            out.at<cv::Vec<ushort, 5> >(i,j) = pixel;
        }
    }
    return out;
}

void setupGeoTransform(QVector<double> * geoTransform)
{
    (*geoTransform).resize(6);
    (*geoTransform)[0] = 1.358847; // Origin X
    (*geoTransform)[3] = 43.575298;  // Origin Y
    (*geoTransform)[2] = (*geoTransform)[4] = 0.0;
    (*geoTransform)[1] = 0.0001; // Step X
    (*geoTransform)[5] = -0.0001;// Step Y
}

void setupMetadata(QList< QPair<QString,QString> > * metadata)
{
    (*metadata) <<  QPair<QString,QString>("My_MD_1","THIS IS A TEST IMAGE");
    (*metadata) <<  QPair<QString,QString>("My_MD_VERSION","0.0");
    (*metadata) <<  QPair<QString,QString>("My_MD_GEO","Somewhere");
    (*metadata) <<  QPair<QString,QString>("My_MD_Satellite","NA");
}

void setupGeoExtent(QPolygonF * geoExtent, const QVector<double> & geoTransform, int width, int height)
{
    (*geoExtent) << QPointF(geoTransform[0], geoTransform[3]);
    (*geoExtent) << QPointF(geoTransform[0]+geoTransform[1]*(width-1), geoTransform[3]);
    (*geoExtent) << QPointF(geoTransform[0]+geoTransform[1]*(width-1), geoTransform[3]+geoTransform[5]*(height-1));
    (*geoExtent) << QPointF(geoTransform[0], geoTransform[3]+geoTransform[5]*(height-1));
}

//*************************************************************************

void DataProviderTest::initTestCase()
{

    // Register GDAL drivers
    GDALAllRegister();

    //
    QDir::addSearchPath("Input", QCoreApplication::applicationDirPath() +
                        "/../../GeoImageViewer_source/Tests/Data/");

    _noDataValue = (1<<16) - 1;

    // create synthetic image:
    _testMatrices << createSynthMat16UC5(WIDTH, HEIGHT);
    _testMatrices << createSynthMat16UC5(WIDTH, HEIGHT);
    cv::Mat t(150, 150, CV_16UC(5));
    t.setTo(_noDataValue);
    cv::Mat & tt = _testMatrices.last();
    t.copyTo(tt(cv::Rect(100, 100, 150, 150)));

    _projectionStr = Core::getProjectionStrFromGeoCS();

    setupGeoTransform(&_geoTransform);

    setupMetadata(&_metadata);

    // (0,0) -> (w-1,0) -> (w-1,h-1) -> (0,h-1)
    setupGeoExtent(&_geoExtent, _geoTransform, WIDTH, HEIGHT);

    for (int i=0; i<_testMatrices.size();i++)
    {
        QString path = QFileInfo("Input:").absoluteFilePath() + QString("/test_image_%1.tif").arg(i);
        _testFiles << path;
        QVERIFY(Core::writeToFile(path, _testMatrices[i],
                              _projectionStr, _geoTransform,
                              _noDataValue, _metadata));
        QVERIFY(QFile(path).exists());
    }

    _provider = new Core::GDALDataProvider();


    //


}

//*************************************************************************

void DataProviderTest::test_computeMask()
{
    int w=20, h=30;
    cv::Mat data = cv::Mat::zeros(h, w, CV_32FC(5));
    data.setTo(10.0);

    cv::Mat trueMask(h, w, CV_8U, cv::Scalar(255));
    cv::Rect r1(2,3,6,7), r2(6,9,4,5), r3(8,3,10,7), r4(0,0,4,5), r5(13,14,3,4);
    std::vector<cv::Mat> iChannels(data.channels());
    cv::split(data, &iChannels[0]);

    iChannels[0](r1) = Core::ImageDataProvider::NoDataValue;
    iChannels[1](r2) = Core::ImageDataProvider::NoDataValue;
    iChannels[2](r3) = Core::ImageDataProvider::NoDataValue;
    iChannels[3](r4) = Core::ImageDataProvider::NoDataValue;
    iChannels[4](r5) = Core::ImageDataProvider::NoDataValue;

    cv::merge(iChannels, data);

    trueMask(r1) = cv::Mat::zeros(r1.height, r1.width, trueMask.type());
    trueMask(r2) = cv::Mat::zeros(r2.height, r2.width, trueMask.type());
    trueMask(r3) = cv::Mat::zeros(r3.height, r3.width, trueMask.type());
    trueMask(r4) = cv::Mat::zeros(r4.height, r4.width, trueMask.type());
    trueMask(r5) = cv::Mat::zeros(r5.height, r5.width, trueMask.type());

    cv::Mat mask = Core::ImageDataProvider::computeMask(data);

    QVERIFY(Core::isEqual(mask, trueMask));

}

//*************************************************************************

bool checkMatrices(const cv::Mat & m1, const cv::Mat & m2, float nodatavalue)
{
    cv::Mat mask = Core::ImageDataProvider::computeMask(m1);
    cv::Mat mask2 = Core::ImageDataProvider::computeMask(m2, nodatavalue);
    if (!Core::isEqual(mask,mask2))
        return false;

    cv::Mat maskND, mask2ND;
    maskND = computeMaskND(m1, Core::ImageDataProvider::NoDataValue);
    mask2ND = computeMaskND(m2, nodatavalue);

    cv::Mat mm1 = m1.mul(maskND);
    cv::Mat mm2 = m2.mul(mask2ND);

    if (!Core::isEqual(mm1,mm2))
        return false;

    return true;
}

void DataProviderTest::test_GDALDataProvider()
{
    QString path = _testFiles[0];

    QVERIFY(_provider->setup(path));
    cv::Mat m = _provider->getImageData();
    cv::Mat m2;

    _testMatrices[0].convertTo(m2, m.depth());

    QVERIFY(checkMatrices(m,m2,_noDataValue));

    // Check geo info :
    QVERIFY( Core::compareProjections(_provider->fetchProjectionRef(), _projectionStr) );
    QVERIFY( comparePolygons(_provider->fetchGeoExtent(), _geoExtent) );
    QVERIFY( compareVectors(_provider->fetchGeoTransform(), _geoTransform) );
    QVERIFY( _provider->getPixelExtent() == QRect(0,0,WIDTH,HEIGHT));

    // check ROI extractions :
    m = _provider->getImageData(QRect(-100,-100, 200, 150));
    cv::Mat m3 = cv::Mat(150, 200, m.type());
    m3.setTo(Core::ImageDataProvider::NoDataValue);
    m2(cv::Rect(0,0,100,50)).copyTo(m3(cv::Rect(100,100,100,50)));
    QVERIFY(Core::isEqual(m3,m));



    // check ROI extraction with upsampling resize:
//    Core::displayMat(m,true,"m");
    m = _provider->getImageData(QRect(-100,-100, 200, 150), 512);
//    Core::displayMat(m,true,"new m");
//    cv::Mat m4;
//    cv::resize(m3,m4,cv::Size(m.cols, m.rows), CV_INTER_NN);
//    Core::displayMat(m4,true,"m4");
//    QVERIFY(Core::isEqual(m4,m));


}

//*************************************************************************

void DataProviderTest::test_GDALDataProvider2()
{

    QString path = _testFiles[1];

    QVERIFY(_provider->setup(path));
    cv::Mat m = _provider->getImageData();
    cv::Mat m2;

    _testMatrices[1].convertTo(m2, m.depth());


    cv::Mat mask = Core::ImageDataProvider::computeMask(m);
    cv::Mat mask2 = Core::ImageDataProvider::computeMask(m2, _noDataValue);
    QVERIFY(Core::isEqual(mask,mask2));

    cv::Mat maskND, mask2ND;
    maskND = computeMaskND(m, Core::ImageDataProvider::NoDataValue);
    mask2ND = computeMaskND(m2, _noDataValue);

    m = m.mul(maskND);
    m2 = m2.mul(mask2ND);

    QVERIFY(Core::isEqual(m,m2));

    // Check geo info :
    QVERIFY( Core::compareProjections(_provider->fetchProjectionRef(), _projectionStr) );
    QVERIFY( comparePolygons(_provider->fetchGeoExtent(), _geoExtent) );
    QVERIFY( compareVectors(_provider->fetchGeoTransform(), _geoTransform) );
    QVERIFY( _provider->getPixelExtent() == QRect(0,0,WIDTH,HEIGHT));

}

//*************************************************************************

void DataProviderTest::test_FloatingDataProvider()
{
    QVERIFY(_provider);

    Core::FloatingDataProvider * provider =
            Core::FloatingDataProvider::createDataProvider(_provider, _provider->getPixelExtent());

    QVERIFY(provider);
    cv::Mat mSrc = _provider->getImageData();
    cv::Mat mDst = provider->getImageData();
    QVERIFY(Core::isEqual(mSrc,mDst));


    // Check geo info :
    QVERIFY( Core::compareProjections(_provider->fetchProjectionRef(), provider->fetchProjectionRef()) );
    QVERIFY( comparePolygons(_provider->fetchGeoExtent(), provider->fetchGeoExtent()) );
    QVERIFY( compareVectors(_provider->fetchGeoTransform(), provider->fetchGeoTransform()) );
    QVERIFY( _provider->getPixelExtent() == provider->getPixelExtent() );

    delete provider;
}

//*************************************************************************

void DataProviderTest::test_FloatingDataProvider2()
{
    QVERIFY(_provider);
    QRect pe = _provider->getPixelExtent().adjusted(-10, -20, -30, -40);
//    QRect pe = _provider->getPixelExtent().adjusted(100, 200, -500, -600);

    Core::FloatingDataProvider * provider =
            Core::FloatingDataProvider::createDataProvider(_provider, pe);

    QVERIFY(provider);
    cv::Mat mSrc = _provider->getImageData();
    cv::Mat mDst = provider->getImageData();

    pe = _provider->getPixelExtent().intersected(pe);
    cv::Rect r(pe.x(),pe.y(),pe.width(),pe.height());

    QVERIFY(Core::isEqual(mSrc(r),mDst));

    // IS NOT FINISHED
    // Check geo info :
    QVERIFY( Core::compareProjections(_provider->fetchProjectionRef(), provider->fetchProjectionRef()) );
//    QVERIFY( comparePolygons(_provider->fetchGeoExtent(), provider->fetchGeoExtent()) );
//    QVERIFY( compareVectors(_provider->fetchGeoTransform(), provider->fetchGeoTransform()) );
//    QVERIFY( _provider->getPixelExtent() == provider->getPixelExtent() );

//    delete provider;
}

//*************************************************************************

void DataProviderTest::test_FloatingDataProvider3()
{
    Core::FloatingDataProvider * provider =
            Core::FloatingDataProvider::createDataProvider("provider", _testMatrices[0]);

    QVERIFY(provider);
    cv::Mat m = provider->getImageData();
    cv::Mat m2;
    _testMatrices[0].convertTo(m2, m.depth());
    delete provider;
    QVERIFY(Core::isEqual(m,m2));

}

//*************************************************************************
/*
void DataProviderTest::test_FloatingDataProvider4()
{
    Core::FloatingDataProvider * provider =
            Core::FloatingDataProvider::createEmptyDataProvider("test", 100, 200);

    QVERIFY(provider);
    cv::Mat m = provider->getImageData();
//    Core::displayMat(m,true,"m");

    cv::Mat m3(20,20,CV_32F,cv::Scalar(15));
    provider->setImageData(QPoint(50,50), m3);
    m = provider->getImageData();
//    Core::displayMat(m,true,"m2");

}
*/
//*************************************************************************

void DataProviderTest::cleanupTestCase()
{
    if (_provider) delete _provider;

    // Remove temp test image
    foreach (QString path, _testFiles)
    {
        QVERIFY(QFile(path).exists());
        QVERIFY(QFile(path).remove());

        QString path2=path+".ovr";
        if(QFile(path2).exists())
            QVERIFY(QFile(path2).remove());

        path2=path+".aux.xml";
        if(QFile(path2).exists())
            QVERIFY(QFile(path2).remove());
    }

    // gdal
    GDALDestroyDriverManager();

}

//*************************************************************************

}

QTEST_MAIN(Tests::DataProviderTest)
