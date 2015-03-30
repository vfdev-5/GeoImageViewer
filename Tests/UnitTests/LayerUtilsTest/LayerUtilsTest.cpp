
// Qt
#include <QDir>
#include <QFileInfo>
#include <QThreadPool>

// OpenCV
#include <opencv2/core/core.hpp>

// Tests
#include "LayerUtilsTest.h"
#include "Core/LayerUtils.h"
#include "Core/ImageDataProvider.h"

namespace Tests
{

int WIDTH  = 2000;
int HEIGHT = 2000;
int DEPTH  = CV_16U;
//int DEPTH2  = CV_16U;
cv::Mat TEST_MATRIX;
QString PROJECTION_STR;
QVector<double> GEO_TRANSFORM;
double NO_DATA_VALUE=(1<<16) - 1;
QList< QPair<QString,QString> > METADATA;
QPolygonF GEO_EXTENT;

//cv::Mat TEST_MATRIX2;


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

void LayerUtilsTest::initTestCase()
{
    QDir::addSearchPath("Input", QCoreApplication::applicationDirPath() +
                        "/../../GeoImageViewer_source/Tests/Data/");

    // Register GDAL drivers
    GDALAllRegister();

    // create synthetic image:
    TEST_MATRIX = cv::Mat(WIDTH, HEIGHT, DEPTH, cv::Scalar(0));
    for (int i=0; i<TEST_MATRIX.rows;i++)
    {
        for (int j=0; j<TEST_MATRIX.cols;j++)
        {
            TEST_MATRIX.at<ushort>(i,j) = (ushort) ( 100 + 1.5*i + 3.4*j +
                                           (TEST_MATRIX.rows-1-i)*j*0.01 +
                                           i*(TEST_MATRIX.rows-1-i)*(TEST_MATRIX.cols*0.5-1-j)*0.001 +
                                           j*j*0.002);
        }
    }

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
}

//*************************************************************************

void LayerUtilsTest::test_GeoComputationMethods()
{
    QVERIFY(Core::isGeoProjection(PROJECTION_STR));
    QVERIFY(!Core::isGeoProjection("PROJECTION_STR"));

    QVERIFY(Core::compareProjections(PROJECTION_STR, PROJECTION_STR));
    QVERIFY(!Core::compareProjections(PROJECTION_STR, "PROJECTION_STR"));

    QVERIFY(Core::isEqual(TEST_MATRIX,TEST_MATRIX));
    QVERIFY(!Core::isEqual(TEST_MATRIX,TEST_MATRIX+2.0));

    QRect pixelExtent = QRect(0,0,WIDTH,HEIGHT);
    QVector<double> gt = Core::computeGeoTransform(GEO_EXTENT, pixelExtent);
    QVERIFY(compareVectors(gt, GEO_TRANSFORM));

}

//*************************************************************************

void LayerUtilsTest::test_FileReadWriteMethods()
{

    QString out = QFileInfo("Input:").absoluteFilePath() + "/test_image0.tif";
    bool res = Core::writeToFile(out, TEST_MATRIX,
                                 PROJECTION_STR, GEO_TRANSFORM,
                                 NO_DATA_VALUE, METADATA);
    QVERIFY(res);
    QVERIFY(QFile(out).exists());


    Core::GDALDataProvider * provider = new Core::GDALDataProvider();
    QVERIFY(provider->setup(out));
    cv::Mat m = provider->getImageData();
    cv::Mat m2;
    TEST_MATRIX.convertTo(m2, m.depth());
    //    Core::displayMat(m, true, "m");
    //    Core::displayMat(m2, true, "m2");
    QVERIFY(Core::isEqual(m,m2));
    // Check geo info :
    QVERIFY( Core::compareProjections(provider->fetchProjectionRef(), PROJECTION_STR) );
    QVERIFY( comparePolygons(provider->fetchGeoExtent(), GEO_EXTENT) );
    QVERIFY( compareVectors(provider->fetchGeoTransform(), GEO_TRANSFORM) );
    QVERIFY( provider->getPixelExtent() == QRect(0,0,WIDTH,HEIGHT));
    delete provider;

    QVERIFY(QFile(out).remove());
}

//*************************************************************************

void LayerUtilsTest::cleanupTestCase()
{
    GDALDestroyDriverManager();
}

//*************************************************************************

}

QTEST_MAIN(Tests::LayerUtilsTest)
