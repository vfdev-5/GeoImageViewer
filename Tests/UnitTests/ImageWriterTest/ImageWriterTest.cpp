
// Qt
#include <QDir>
#include <QFileInfo>
#include <QThreadPool>

// OpenCV
#include <opencv2/core/core.hpp>

// Tests
#include "ImageWriterTest.h"
#include "Core/LayerUtils.h"
#include "Core/ImageDataProvider.h"
#include "Core/FloatingDataProvider.h"

namespace Tests
{

int WIDTH  = 2000;
int HEIGHT = 2000;
int DEPTH  = CV_16U;
//int DEPTH2  = CV_16U;
cv::Mat TEST_MATRIX;
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

//    Core::displayMat(TEST_MATRIX, true, "TEST_MATRIX", false);

    // Create a Floating image data provider
    _provider = Core::FloatingDataProvider::createDataProvider("test", TEST_MATRIX);
    QVERIFY(_provider);
    _provider->setParent(this);

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

    QVERIFY(_imageWriter->write(out, _provider));
    QVERIFY(QFile(out).exists());

    Core::GDALDataProvider * provider = new Core::GDALDataProvider();
    QVERIFY(provider->setup(out));

    cv::Mat m = _provider->getImageData();
    cv::Mat m2 = provider->getImageData();
//    Core::displayMat(m, true, "m");
//    Core::displayMat(m2, true, "m2");
    QVERIFY(Core::isEqual(m,m2));
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
    _imageWriter->writeInBackground(out, _provider);

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
