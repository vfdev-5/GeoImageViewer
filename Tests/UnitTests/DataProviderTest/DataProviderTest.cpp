
// Qt
#include <QDir>
#include <QFileInfo>
#include <QThreadPool>

// OpenCV
#include <opencv2/core/core.hpp>

// Tests
#include "DataProviderTest.h"
#include "Core/LayerUtils.h"

namespace Tests
{

int WIDTH  = 2000;
int HEIGHT = 2000;
int DEPTH  = CV_16UC4;
cv::Mat TEST_MATRIX;

QString PROJECTION_STR;
QVector<double> GEO_TRANSFORM;
double NO_DATA_VALUE=(1<<16) - 1;
QList< QPair<QString,QString> > METADATA;
QPolygonF GEO_EXTENT;


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


//*************************************************************************

void DataProviderTest::initTestCase()
{

    // Register GDAL drivers
    GDALAllRegister();

    // create synthetic image:
    TEST_MATRIX = cv::Mat(WIDTH, HEIGHT, DEPTH, cv::Scalar(0));
    for (int i=0; i<TEST_MATRIX.rows;i++)
    {
        for (int j=0; j<TEST_MATRIX.cols;j++)
        {
            ushort c1 = (ushort) ( 100 + 1.5*i + 3.4*j +
                                   (TEST_MATRIX.rows-1-i)*j*0.01 +
                                   i*(TEST_MATRIX.rows-1-i)*(TEST_MATRIX.cols*0.5-1-j)*0.001 +
                                   j*j*0.002);
            ushort c2 = (ushort) ( 10 + 2.5*i - 1.4*j +
                                   (TEST_MATRIX.rows-1-i)*j*0.02 +
                                   i*(TEST_MATRIX.rows-1-i)*(TEST_MATRIX.cols*0.3-1-j)*0.01 +
                                   j*j*0.0012);
            ushort c3 = (ushort) ( 1.5*i + 5.4*j +
                                   (TEST_MATRIX.rows-1-i)*j*0.01 +
                                   i*(TEST_MATRIX.rows-1-i)*(TEST_MATRIX.cols*0.5-1-j)*0.001 +
                                   j*j*0.002);
            ushort c4 = (ushort) ( 50 + 1.5*i + 4.4*j +
                                   (TEST_MATRIX.rows*0.5-1-i)*j*0.01 +
                                   i*(TEST_MATRIX.rows-1-i)*(TEST_MATRIX.cols*0.7-1-j)*0.001 +
                                   j*j*0.01);

            cv::Vec4s pixel = cv::Vec4s(c1,c2,c3,c4);
            TEST_MATRIX.at<cv::Vec4s>(i,j) = pixel;
        }
    }

//    Core::displayMat(TEST_MATRIX, true, "TEST_MATRIX");

    QDir::addSearchPath("Input", QCoreApplication::applicationDirPath() +
                        "/../../GeoImageViewer_source/Tests/Data/");
    QString path = QFileInfo("Input:").absoluteFilePath() + "/test_image.tif";

    PROJECTION_STR = Core::getProjectionStrFromGeoCS();

    GEO_TRANSFORM.resize(6);
    GEO_TRANSFORM[0] = 1.358847; // Origin X
    GEO_TRANSFORM[3] = 43.575298;  // Origin Y
    GEO_TRANSFORM[2] = GEO_TRANSFORM[4] = 0.0;
    GEO_TRANSFORM[1] = 0.0001; // Step X
    GEO_TRANSFORM[5] = -0.0001;// Step Y
    METADATA <<  QPair<QString,QString>("My_MD_1","THIS IS A TEST IMAGE");
    METADATA <<  QPair<QString,QString>("My_MD_VERSION","0.0");
    METADATA <<  QPair<QString,QString>("My_MD_GEO","Somewhere");
    METADATA <<  QPair<QString,QString>("My_MD_Satellite","NA");
    // (0,0) -> (w-1,0) -> (w-1,h-1) -> (0,h-1)
    GEO_EXTENT << QPointF(GEO_TRANSFORM[0], GEO_TRANSFORM[3]);
    GEO_EXTENT << QPointF(GEO_TRANSFORM[0]+GEO_TRANSFORM[1]*(WIDTH-1), GEO_TRANSFORM[3]);
    GEO_EXTENT << QPointF(GEO_TRANSFORM[0]+GEO_TRANSFORM[1]*(WIDTH-1), GEO_TRANSFORM[3]+GEO_TRANSFORM[5]*(HEIGHT-1));
    GEO_EXTENT << QPointF(GEO_TRANSFORM[0], GEO_TRANSFORM[3]+GEO_TRANSFORM[5]*(HEIGHT-1));

    QVERIFY(Core::writeToFile(path, TEST_MATRIX,
                              PROJECTION_STR, GEO_TRANSFORM,
                              NO_DATA_VALUE, METADATA));

    QVERIFY(QFile(path).exists());

}

//*************************************************************************

void DataProviderTest::test()
{
    QString path = QFileInfo("Input:").absoluteFilePath() + "/test_image.tif";
    _provider = new Core::GDALDataProvider();
    QVERIFY(_provider->setup(path));
    cv::Mat m = _provider->getImageData();
    cv::Mat m2;
    TEST_MATRIX.convertTo(m2, m.depth());
    QVERIFY(Core::isEqual(m,m2));

    // Check geo info :
    QVERIFY( Core::compareProjections(_provider->fetchProjectionRef(), PROJECTION_STR) );
    QVERIFY( comparePolygons(_provider->fetchGeoExtent(), GEO_EXTENT) );
    QVERIFY( compareVectors(_provider->fetchGeoTransform(), GEO_TRANSFORM) );
    QVERIFY( _provider->getPixelExtent() == QRect(0,0,WIDTH,HEIGHT));

}

//*************************************************************************

void DataProviderTest::test2()
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

void DataProviderTest::test2_1()
{
    QVERIFY(_provider);

    // IS NOT FINISHED
//    QRect pe = _provider->getPixelExtent().adjusted(-10, -20, -30, -40);

//    Core::FloatingDataProvider * provider =
//            Core::FloatingDataProvider::createDataProvider(_provider, pe);

//    QVERIFY(provider);
//    cv::Mat mSrc = _provider->getImageData();
//    cv::Mat mDst = provider->getImageData();
//    cv::Rect r(pe.x(),pe.y(),pe.width(),pe.height());
//    QVERIFY(Core::isEqual(mSrc(r),mDst));


//    // Check geo info :
//    QVERIFY( Core::compareProjections(_provider->fetchProjectionRef(), provider->fetchProjectionRef()) );
//    QVERIFY( _provider->fetchGeoExtent() == provider->fetchGeoExtent() );
//    QVERIFY( _provider->fetchGeoTransform() == provider->fetchGeoTransform() );
//    QVERIFY( _provider->getPixelExtent() == provider->getPixelExtent() );

//    delete provider;
}

//*************************************************************************

void DataProviderTest::test3()
{
    Core::FloatingDataProvider * provider =
            Core::FloatingDataProvider::createDataProvider("provider", TEST_MATRIX);

    QVERIFY(provider);
    cv::Mat m = provider->getImageData();
    cv::Mat m2;
    TEST_MATRIX.convertTo(m2, m.depth());
    delete provider;
    QVERIFY(Core::isEqual(m,m2));

}

//*************************************************************************

void DataProviderTest::cleanupTestCase()
{
    if (_provider) delete _provider;

    // Remove temp test image
    QString path = QFileInfo("Input:").absoluteFilePath() + "/test_image.tif";
    QVERIFY(QFile(path).exists());
    QVERIFY(QFile(path).remove());

    QString path2=path+".ovr";
    if(QFile(path2).exists())
        QVERIFY(QFile(path2).remove());

    path2=path+".aux.xml";
    if(QFile(path2).exists())
        QVERIFY(QFile(path2).remove());


    // gdal
    GDALDestroyDriverManager();

}

//*************************************************************************

}

QTEST_MAIN(Tests::DataProviderTest)
