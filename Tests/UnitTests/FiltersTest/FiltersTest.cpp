
// Qt
#include <QDir>
#include <QFileInfo>
#include <QThreadPool>

// OpenCV
#include <opencv2/core/core.hpp>

// Tests
#include "FiltersTest.h"
#include "Core/LayerUtils.h"
#include "Filters/BlurFilter.h"

namespace Tests
{

int WIDTH  = 2000;
int HEIGHT = 2000;
int DEPTH  = CV_16UC4;
cv::Mat TEST_MATRIX;

//*************************************************************************

void FiltersTest::initTestCase()
{
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
}

//*************************************************************************

void FiltersTest::test_exception()
{
    // test opencv exception handling

    Filters::BlurFilter bf;
    bf.setSizeX(-1);
    bf.setSizeY(-1);

    cv::Mat r = bf.apply(TEST_MATRIX);

    QVERIFY(r.empty());
}

//*************************************************************************

void FiltersTest::test2()
{
    QVERIFY(true);
}

//*************************************************************************

void FiltersTest::test3()
{
    QVERIFY(true);
}

//*************************************************************************

void FiltersTest::cleanupTestCase()
{
    QVERIFY(true);
}

//*************************************************************************

}

QTEST_MAIN(Tests::FiltersTest)
