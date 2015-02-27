
// Qt
#include <QDir>
#include <QFileInfo>
#include <QThreadPool>

// OpenCV
#include <opencv2/core/core.hpp>

// Tests
#include "ColorTest.h"

namespace Tests
{

//*************************************************************************

void ColorTest::test()
{
    cv::Mat m(10, 10, CV_8UC4, cv::Scalar(0));

    // cv::Mat has BGRA format
    m.at<cv::Vec4b>(0,0) = cv::Vec4b(30,20,10,40);
    m.at<cv::Vec4b>(0,1) = cv::Vec4b(3,2,1,5);

    QImage im(m.data,
              m.cols,
              m.rows,
              QImage::Format_ARGB32);

    QColor p;
    p = QColor::fromRgba(im.pixel(0,0));

    QVERIFY(p.red() == 10
            && p.green() == 20
            && p.blue() == 30
            && p.alpha() == 40);

    p = QColor::fromRgba(im.pixel(1,0));

    QVERIFY(p.red() == 1
            && p.green() == 2
            && p.blue() == 3
            && p.alpha() == 5
            );

    /// QColor and QRgb
    uint v;
    for (int i=0; i<255; i++)
    {
        // QColor::setRgb(int r, int g, int b, int a) stored as ct.argb {ushort alpha, red, green, blue}
        ushort s = i * 0x101; // <-> i * 257
        int ii = s >> 8; // <-> s / 2^8
        if (ii != i) {
            QVERIFY(false);
        }
        // QRgb=QColor::rgba() , QRgb == uint
        uint b = (i & 0xff); // <-> mod(i, 256)
        uint g = (i & 0xff) << 8; // <-> ( mod(i, 256) ) * 2^8
        uint r = (i & 0xff) << 16; // <-> ( mod(i, 256) ) * 2^16
        uint a = (i & 0xff) << 24; // <-> ( mod(i, 256) ) * 2^24
        // ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff)
        v = a | r | g | b;

        // uint -> r,g,b,a
        QVERIFY( i == (v >> 24));
        QVERIFY( i == ((v >> 16) & 0xff));
        QVERIFY( i == ((v >> 8) & 0xff));
        QVERIFY( i == (v & 0xff));
    }

    v = 255 & 0xff;
    v = 256 & 0xff;
    v = 257 & 0xff;



    //
//    double alpha = 0.001;
//    double nvalue = 255 * alpha + (1.0 - alpha) * 255;
//    uchar u = (uchar) nvalue;
//    int a = 0;
//    QVERIFY(true);


}

//*************************************************************************

}

QTEST_MAIN(Tests::ColorTest)
