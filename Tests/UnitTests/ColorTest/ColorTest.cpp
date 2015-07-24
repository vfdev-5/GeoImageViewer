
// Qt
#include <QDir>
#include <QImage>
#include <QLabel>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "Core/LayerUtils.h"

// Tests
#include "../../Common.h"
#include "ColorTest.h"


namespace Tests
{

#define VERBOSE false

//*************************************************************************

void ColorTest::test()
{
    cv::Mat m(10, 10, CV_8UC4, cv::Scalar(0));

    // In Little-endian :
    // cv::Mat stores pixels in the provided order : 30, 20, 10, 40, 3, 2, 1, 5, ...
    m.at<cv::Vec4b>(0,0) = cv::Vec4b(30,20,10,40);
    m.at<cv::Vec4b>(0,1) = cv::Vec4b(3,2,1,5);

    if (VERBOSE)
    {
        uchar * srcPtr = m.data;
        for (int i=0; i<20;i++)
        {
            std::cout << (int) srcPtr[i] << " ";
        }
        std::cout << std::endl;
    }


    // According to Qt docs :
    // QImage::Format_ARGB32 -> The image is stored using a 32-bit ARGB format (0xAARRGGBB).
    QImage im(m.data,
              m.cols,
              m.rows,
              QImage::Format_ARGB32);

    // QImage stores its data as : 30, 20, 10, 40, 3, 2, 1, 5
    if (VERBOSE)
    {
        uchar * srcPtr = im.bits();
        for (int i=0; i<20;i++)
        {
            std::cout << (int) srcPtr[i] << " ";
        }
        std::cout << std::endl;
    }
    // Pixel format is 0xAARRGGBB =>
    QColor p;
    p = QColor::fromRgba(im.pixel(0,0));
    if (VERBOSE) std::cout << "RGBA : " << p.red() << ", " << p.green() << ", " << p.blue() << ", " << p.alpha() << std::endl;
    QVERIFY(p.red() == 10
            && p.green() == 20
            && p.blue() == 30
            && p.alpha() == 40);

    p = QColor::fromRgba(im.pixel(1,0));
    if (VERBOSE) std::cout << "RGBA : " << p.red() << ", " << p.green() << ", " << p.blue() << ", " << p.alpha() << std::endl;
    QVERIFY(p.red() == 1
            && p.green() == 2
            && p.blue() == 3
            && p.alpha() == 5
            );

    // If tests are passed : pixels are interpreted as BGRA

    cv::Mat m2;
    cv::cvtColor(m, m2, cv::COLOR_RGBA2BGRA);

    if (VERBOSE)
    {
        uchar * srcPtr = m2.data;
        for (int i=0; i<20;i++)
        {
            std::cout << (int) srcPtr[i] << " ";
        }
        std::cout << std::endl;
    }

    QImage im2(m2.data,
              m2.cols,
              m2.rows,
              QImage::Format_ARGB32);



    p = QColor::fromRgba(im2.pixel(0,0));
    if (VERBOSE) std::cout << "RGBA : " << p.red() << ", " << p.green() << ", " << p.blue() << ", " << p.alpha() << std::endl;
    QVERIFY(p.red() == 30
            && p.green() == 20
            && p.blue() == 10
            && p.alpha() == 40);

    p = QColor::fromRgba(im2.pixel(1,0));
    if (VERBOSE) std::cout << "RGBA : " << p.red() << ", " << p.green() << ", " << p.blue() << ", " << p.alpha() << std::endl;
    QVERIFY(p.red() == 3
            && p.green() == 2
            && p.blue() == 1
            && p.alpha() == 5
            );


    // QImage::Format_RGBA8888 -> The image is stored using a 32-bit byte-ordered RGBA format (8-8-8-8).
    // Unlike ARGB32 this is a byte-ordered format, which means the 32bit encoding differs between big
    // endian and little endian architectures, being respectively (0xRRGGBBAA) and (0xAABBGGRR).
    // The order of the colors is the same on any architecture if read as bytes 0xRR,0xGG,0xBB,0xAA.

    QImage im3(m.data,
              m.cols,
              m.rows,
              QImage::Format_RGBA8888);

    // Pixel format is 0xRRGGBBAA (b-e) and 0xAABBGGRR (l-e)
    p = QColor::fromRgba(im3.pixel(0,0));
    std::cout << "RGBA : " << p.red() << ", " << p.green() << ", " << p.blue() << ", " << p.alpha() << std::endl;
    p = QColor::fromRgba(im3.pixel(1,0));
    std::cout << "RGBA : " << p.red() << ", " << p.green() << ", " << p.blue() << ", " << p.alpha() << std::endl;




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

    //    double alpha = 0.001;
    //    double nvalue = 255 * alpha + (1.0 - alpha) * 255;
    //    uchar u = (uchar) nvalue;
    //    int a = 0;
    //    QVERIFY(true);


}

//*************************************************************************

void ColorTest::test2()
{
    cv::Mat m(10, 10, CV_8U, cv::Scalar(0));

    // cv::Mat has BGRA format
    m.at<uchar>(0,0) = 10;
    m.at<uchar>(0,1) = 20;
    m.at<uchar>(1,0) = 30;
    m.at<uchar>(1,1) = 40;

    // a)
    std::vector<cv::Mat> oChannels(3);
    oChannels[0] = oChannels[1] = oChannels[2] = m;
    cv::Mat out;
    cv::merge(oChannels, out);
//    Core::printMat(out, "out");

    QVERIFY(out.channels() == 3);
    QVERIFY(out.rows == 10);
    QVERIFY(out.cols == 10);
    QVERIFY(out.type() == CV_8UC3);



}

//*************************************************************************

}

QTEST_MAIN(Tests::ColorTest)
