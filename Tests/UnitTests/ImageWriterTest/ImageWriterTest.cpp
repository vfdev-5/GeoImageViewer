
// Qt
#include <QDir>
#include <QFileInfo>
#include <QThreadPool>

// OpenCV
#include <opencv2/core/core.hpp>

// Tests
#include "ImageWriterTest.h"
#include "Core/LayerUtils.h"
#include "Core/FloatingDataProvider.h"

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

//*************************************************************************

void ImageWriterTest::initTestCase()
{
    QDir::addSearchPath("Input", QCoreApplication::applicationDirPath() +
                        "/../../GeoImageViewer_source/Tests/Data/");

    // Register GDAL drivers
    _imageWriter = new Core::ImageWriter(this);

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

//    QString p1 = Core::getProjectionStrFromEPSG();
//    QString p2 = Core::getProjectionStrFromGeoCS();
//    QVERIFY(p1 == p2);
    PROJECTION_STR = Core::getProjectionStrFromGeoCS();

    QVERIFY(Core::isGeoProjection(PROJECTION_STR));
    QVERIFY(Core::compareProjections(PROJECTION_STR, PROJECTION_STR));

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

//    Core::displayMat(TEST_MATRIX, true, "TEST_MATRIX", false);

    // Create a Floating image data provider
    _provider = Core::FloatingDataProvider::createDataProvider("test", TEST_MATRIX);
    QVERIFY(_provider);
    _provider->setParent(this);

    _provider->setProjectionRef(PROJECTION_STR);
    _provider->setGeoTransform(GEO_TRANSFORM);
    _provider->setGeoExtent(GEO_EXTENT);

//    QRect pixelExtent = QRect(0,0,WIDTH,HEIGHT);
//    QVector<double> gt = Core::computeGeoTransform(GEO_EXTENT, pixelExtent);
//    QVERIFY(gt == GEO_TRANSFORM);


    // Create a GeoItemLayer to store geo image info :
    _geoInfo = new Core::GeoImageLayer(this);
    _geoInfo->setType("Image");
    _geoInfo->setImageName(_provider->getImageName());
    _geoInfo->setNbBands(_provider->getInputNbBands());
    _geoInfo->setDepthInBytes(_provider->getInputDepthInBytes());
    _geoInfo->setIsComplex(_provider->inputIsComplex());
    _geoInfo->setGeoExtent(_provider->fetchGeoExtent());
    _geoInfo->setGeoBBox(_geoInfo->getGeoExtent().boundingRect());
    _geoInfo->setPixelExtent(_provider->getPixelExtent());
    _geoInfo->setProjectionRef(_provider->fetchProjectionRef());
    _geoInfo->setGeoTransform(_provider->fetchGeoTransform());

    // check createDataProvider method
    cv::Mat m = _provider->getImageData();
//    Core::displayMat(m, true, "m");
    cv::Mat m2;
    TEST_MATRIX.convertTo(m2, m.depth());
    QVERIFY(Core::isEqual(m,m2));
}

//*************************************************************************

void ImageWriterTest::test()
{
    QString out = QFileInfo("Input:").absoluteFilePath() + "/test_image.tif";

    QVERIFY(_imageWriter->write(out, _provider, _geoInfo));
    QVERIFY(QFile(out).exists());

    Core::GDALDataProvider * provider = new Core::GDALDataProvider();
    QVERIFY(provider->setup(out));

    cv::Mat m = _provider->getImageData();
    cv::Mat m2 = provider->getImageData();
//    Core::displayMat(m, true, "m");
//    Core::displayMat(m2, true, "m2");
    QVERIFY(Core::isEqual(m,m2));

    // test geo info:
    QVERIFY( Core::compareProjections(provider->fetchProjectionRef(), _geoInfo->getProjectionRef()) );
    QVERIFY( provider->fetchGeoExtent() == _geoInfo->getGeoExtent() );
    QVERIFY( provider->fetchGeoTransform() == _geoInfo->getGeoTransform() );

    delete provider;
    QVERIFY(QFile(out).remove());
}

//*************************************************************************

void ImageWriterTest::test2()
{
    QString out = QFileInfo("Input:").absoluteFilePath() + "/test_image_2.tif";

    connect(_imageWriter, &Core::ImageWriter::imageWriteFinished,
            this, &Tests::ImageWriterTest::onImageWriteFinished);
    writeFinished = false;
    _imageWriter->writeInBackground(out, _provider, _geoInfo);

    int i = 0;
    while (!writeFinished && i++ < 50)
        QTest::qWait(500);
}

//*************************************************************************

void ImageWriterTest::onImageWriteFinished(bool ok)
{
    writeFinished = true;
    QVERIFY(ok);

    QString out = QFileInfo("Input:").absoluteFilePath() + "/test_image_2.tif";

    Core::GDALDataProvider * provider = new Core::GDALDataProvider();
    QVERIFY(provider->setup(out));

    cv::Mat m = _provider->getImageData();
    cv::Mat m2 = provider->getImageData();
//    Core::displayMat(m, true, "m");
//    Core::displayMat(m2, true, "m2");
    QVERIFY(Core::isEqual(m,m2));

    // test geo info:
    QVERIFY( Core::compareProjections(provider->fetchProjectionRef(), _geoInfo->getProjectionRef()) );
    QVERIFY( provider->fetchGeoExtent() == _geoInfo->getGeoExtent() );
    QVERIFY( provider->fetchGeoTransform() == _geoInfo->getGeoTransform() );


    delete provider;

    QVERIFY(QFile(out).remove());

    disconnect(_imageWriter, &Core::ImageWriter::imageWriteFinished,
            this, &Tests::ImageWriterTest::onImageWriteFinished);

}

//*************************************************************************

void ImageWriterTest::cleanupTestCase()
{

}

//*************************************************************************

}

QTEST_MAIN(Tests::ImageWriterTest)
