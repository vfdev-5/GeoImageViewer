
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

    // Create a Floating image data provider
    _provider = Core::FloatingDataProvider::createDataProvider("test", TEST_MATRIX);
    QVERIFY(_provider);
    _provider->setParent(this);

    // check createDataProvider method
    cv::Mat m = _provider->getImageData();
    cv::Mat res;
    TEST_MATRIX.convertTo(res, m.depth());
    res -= m;
    QVERIFY(cv::countNonZero(res) == 0);

}

//*************************************************************************

void ImageWriterTest::test()
{

    QString out = QFileInfo("Input:").absoluteFilePath() + "/test_image.tif";

    QVERIFY(_imageWriter->write(out, _provider));
    QVERIFY(QFile(out).exists());
    QVERIFY(QFile(out).remove());

}


//*************************************************************************

//void ImageWriterTest::test2()
//{
//    connect(_imageOpener, &Core::ImageOpener::imageOpened,
//            this, &Tests::ImageOpenerTest::onImageOpened);
//    imageOpened = false;
//    _imageOpener->openImageInBackground(_url);

//    int i = 0;
//    while (!imageOpened && i++ < 50)
//        QTest::qWait(500);

//}

//*************************************************************************

void ImageWriterTest::onImageWriteFinished(bool ok)
{
//    imageOpened=true;
//    QVERIFY(provider);

//    cv::Mat m = provider->getImageData();
//    cv::Mat res;
//    TEST_MATRIX.convertTo(res, m.depth());
//    res -= m;
//    QVERIFY(cv::countNonZero(res) == 0);

//    delete provider;
}

//*************************************************************************

void ImageWriterTest::cleanupTestCase()
{

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


}

//*************************************************************************

}

QTEST_MAIN(Tests::ImageWriterTest)
