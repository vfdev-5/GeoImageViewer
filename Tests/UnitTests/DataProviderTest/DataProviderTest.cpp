
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

    QVERIFY(Core::writeToFile(path, TEST_MATRIX));

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
    delete provider;
    QVERIFY(Core::isEqual(mSrc,mDst));

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
