
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

    // b)
    //    cv::Mat out2[] = {m,m,m};
    //    Core::printMat(*out2, "out2");

    //    QVERIFY(out2->channels() == 3);
    //    QVERIFY(out2->rows == 10);
    //    QVERIFY(out2->cols == 10);
    //    QVERIFY(out2->type() == CV_8UC3);


}

//*************************************************************************
/*
void ColorTest::test3()
{

    cv::Mat data(100, 100, CV_32F, cv::Scalar(0));

    cv::Mat i1(10,20,CV_32F,cv::Scalar(10));
    i1.copyTo(data(cv::Rect(10,5,20,10)));

    cv::Mat i2(20,30,CV_32F,cv::Scalar(20));
    i2.copyTo(data(cv::Rect(30,55,30,20)));

    cv::Mat i3(10,10,CV_32F,cv::Scalar(30));
    i3.copyTo(data(cv::Rect(60,15,10,10)));

    cv::Mat i4(10,20,CV_32F,cv::Scalar(40));
    i4.copyTo(data(cv::Rect(70,45,20,10)));

    Core::displayMat(data,true,"data");

    int _threshold = 20;



    cv::Mat in;
    if (data.channels() > 1)
    {
        std::vector<cv::Mat> iChannels(data.channels());
        cv::split(data, &iChannels[0]);
        in = iChannels[0];
    }
    else
    {
        in = data;
    }

    cv::Mat processedData;
    cv::threshold(in, processedData, _threshold, 255, cv::THRESH_BINARY);

    cv::Mat out, processedData8U;
    if (processedData.depth() != CV_8U)
    {
        processedData.convertTo(processedData8U, CV_8U);
    }
    else
    {
        processedData8U = processedData;
    }
    std::vector<cv::Mat> oChannels(3);
    oChannels[0] = oChannels[1] = oChannels[2] = processedData8U;
    cv::merge(oChannels, out);


    QImage im = QImage(out.data,
                  out.cols,
                  out.rows,
                  QImage::Format_RGB888);

    QLabel * label = new QLabel();
    label->setPixmap(QPixmap::fromImage(im));
    label->show();

    Core::displayMat(out, true, "out");


}
*/
//*************************************************************************

}

QTEST_MAIN(Tests::ColorTest)
