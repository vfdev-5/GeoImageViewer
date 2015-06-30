
// Qt
#include <QDir>
#include <QFileInfo>
#include <QThreadPool>

// OpenCV
#include <opencv2/core/core.hpp>

// Tests
#include "ImageOpenerTest.h"
#include "Core/LayerUtils.h"

namespace Tests
{

int WIDTH  = 2000;
int HEIGHT = 2000;
int DEPTH  = CV_16U;
//int DEPTH2  = CV_16U;
cv::Mat TEST_MATRIX;
//cv::Mat TEST_MATRIX2;

//*************************************************************************

void ImageOpenerTest::initTestCase()
{

    // Register GDAL drivers
    _imageOpener = new Core::ImageOpener(this);

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

//    Core::displayMat(TEST_MATRIX, true, "TEST_MATRIX");

    // Create temporary dir
    QDir d = QDir::currentPath() + "/Temp_tests_123";
    SD_TRACE("Temporary dir : " + d.absolutePath());
    if (!d.exists())
    {
        d.mkdir(d.absolutePath());
    }
    QDir::addSearchPath("Input", d.absolutePath());


    QString path = QFileInfo("Input:").absoluteFilePath() + "/test_image.tif";

    QVERIFY(Core::writeToFile(path, TEST_MATRIX));

    _url = QUrl("file:///" + path);

    QVERIFY(_url.isLocalFile());
    QVERIFY(_url.isValid());

}

//*************************************************************************

void ImageOpenerTest::test_openImage()
{

    Core::ImageDataProvider * provider = _imageOpener->openImage(_url);
    QVERIFY(provider);

    cv::Mat m = provider->getImageData();
    cv::Mat res;
    TEST_MATRIX.convertTo(res, m.depth());
    res -= m;
    delete provider;
    QVERIFY(cv::countNonZero(res) == 0);
}


//*************************************************************************

void ImageOpenerTest::test_openImageInBackground()
{
    connect(_imageOpener, &Core::ImageOpener::imageOpened,
            this, &Tests::ImageOpenerTest::onImageOpened);
    imageOpened = false;
    _imageOpener->openImageInBackground(_url);

    int i = 0;
    while (!imageOpened && i++ < 50)
        QTest::qWait(500);

}

//*************************************************************************

void ImageOpenerTest::test_openImageInBackground_and_Cancel()
{
    connect(_imageOpener, &Core::ImageOpener::imageOpened,
            this, &Tests::ImageOpenerTest::onImageOpenCanceled);
    imageOpened = false;
    _imageOpener->openImageInBackground(_url);

    int i = 0;
    while (!imageOpened && i++ < 50)
    {
        _imageOpener->cancel();
        QTest::qWait(500);
    }

}

//*************************************************************************

void ImageOpenerTest::onImageOpenCanceled(Core::ImageDataProvider * provider)
{
    imageOpened=true;
    QVERIFY(provider == 0);

    disconnect(_imageOpener, &Core::ImageOpener::imageOpened,
            this, &Tests::ImageOpenerTest::onImageOpenCanceled);
}

//*************************************************************************

void ImageOpenerTest::onImageOpened(Core::ImageDataProvider * provider)
{
    imageOpened=true;
    QVERIFY(provider);

    cv::Mat m = provider->getImageData();
    cv::Mat res;
    TEST_MATRIX.convertTo(res, m.depth());
    res -= m;
    QVERIFY(cv::countNonZero(res) == 0);

    delete provider;

    disconnect(_imageOpener, &Core::ImageOpener::imageOpened,
            this, &Tests::ImageOpenerTest::onImageOpened);
}

//*************************************************************************

void ImageOpenerTest::cleanupTestCase()
{

    // remove temporary directory:
    QDir d("Input:");
    QVERIFY(d.removeRecursively());

}

//*************************************************************************

}

QTEST_MAIN(Tests::ImageOpenerTest)
