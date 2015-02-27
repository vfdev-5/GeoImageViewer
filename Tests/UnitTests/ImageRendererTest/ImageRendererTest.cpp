
// Qt
#include <QImage>
#include <QColor>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Tests
#include "ImageRendererTest.h"
#include "Core/HistogramImageRenderer.h"
#include "Core/ImageDataProvider.h"

namespace Tests
{

bool testRGBAColor(const cv::Vec4b & p, int r, int g, int b, int a)
{
    return p[0] == r && p[1] == g && p[2] == b && p[3] == a;
}

bool testBGRAColor(const cv::Vec4b & p, int r, int g, int b, int a)
{
    return p[2] == r && p[1] == g && p[0] == b && p[3] == a;
}


bool testRGBAColor2(const QColor & p, int r, int g, int b, int a)
{
    return p.red() == r && p.green() == g && p.blue() == b && p.alpha() == a;
}

//*************************************************************************
/*!
 * \brief ImageRendererTest::test verifies the use of rendering with HistogramImageRenderer of 1 band non complex imagery.
 *
 */
void ImageRendererTest::test()
{
    // Define raw data : 1 band, nonComplex imagery
    cv::Mat m(3, 5, CV_32F, cv::Scalar(0));

    // cv::Mat has BGRA format
    m.at<float>(0,0) = 0.0;
    m.at<float>(0,1) = 100.0;
    m.at<float>(0,2) = 200.0;
    m.at<float>(0,3) = 300.0;
    m.at<float>(0,4) = 410.0;

    m.at<float>(1,0) = 10.0;
    m.at<float>(1,1) = 110.0;
    m.at<float>(1,2) = 220.0;
    m.at<float>(1,3) = 330.0;
    m.at<float>(1,4) = 340.0;
    // noDataValues :
    m.at<float>(2,0) = Core::ImageDataProvider::NoDataValue;
    m.at<float>(2,1) = Core::ImageDataProvider::NoDataValue;
    m.at<float>(2,2) = Core::ImageDataProvider::NoDataValue;

    // Define renderer:
    Core::HistogramImageRenderer renderer;
    Core::ImageRendererConfiguration conf;
    conf.minValues << 0.0;
    conf.maxValues << 400.0;
    conf.toRGBMapping.insert(0,0);
    conf.toRGBMapping.insert(1,0);
    conf.toRGBMapping.insert(2,0);
    renderer.setConfiguration(conf);
    Core::HistogramRendererConfiguration hConf;
    hConf.qMinValues << 0.0;
    hConf.qMaxValues << 400.0;
    hConf.transferFunctions << Core::HistogramRendererConfiguration::availableTransferFunctions[0];
    hConf.isDiscreteValues << false;
    // define histogram stops as :
    QGradientStops stops;
    // all below 100.0 is black,
    stops << QGradientStop(0.25, Qt::black);
    // between 100.0 and 200.0 is gradiend to red
    stops << QGradientStop(0.5, Qt::red);
    // between 200.0 and 300.0 is gradiend to blue
    stops << QGradientStop(0.75, Qt::blue);
    // above 400.0 is gradiend to white
    stops << QGradientStop(1.0, Qt::white);
    hConf.normHistStops << stops;
    renderer.setHistConfiguration(hConf);


    // Render raw data
    cv::Mat r = renderer.render(m, true);

#ifdef _DEBUG
    std::cout << "Display matrix value :" << std::endl;
    for (int i=0; i<r.rows; i++) {
        std::cout << i << " : ";
        for (int j=0; j<r.cols; j++) {
            cv::Vec4b p = r.at<cv::Vec4b>(i,j);
            std::cout << "(" << (int) p[0] << "|" << (int) p[1] << "|" << (int) p[2] << "|" << (int) p[3] << "), ";
        }
        std::cout << std::endl;
    }
#endif

    // Convert to QImage
    // For correct conversion : rendered image should be in BGRA format
    QImage im(r.data,
              r.cols,
              r.rows,
              QImage::Format_ARGB32);

    // Test colors:
//    m.at<float>(0,0) = 0.0; -> black | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,0),0,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,0)),0,0,0,255));

//    m.at<float>(0,1) = 100.0; -> black | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,1),0,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,0)),0,0,0,255));

//    m.at<float>(0,2) = 200.0; -> red | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,2),255,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,0)),255,0,0,255));

//    m.at<float>(0,3) = 300.0; -> blue | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,3),0,0,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(3,0)),0,0,255,255));

//    m.at<float>(0,4) = 400.0;
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,4),255,255,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(4,0)),255,255,255,255));

//    m.at<float>(1,0) = 10.0; -> black | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,0),0,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,1)),0,0,0,255));

//    m.at<float>(1,1) = 110.0; -> 90% black + 10% red | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,1),25,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,1)),25,0,0,255));

//    m.at<float>(1,2) = 220.0; -> 80% red + 20% blue
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,2),203,0,51,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,1)),203,0,51,255));

//    m.at<float>(1,3) = 330.0;  -> 70% blue + 30% white
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,3),76,76,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(3,1)),76,76,255,255));

//    m.at<float>(1,4) = 340.0; -> 60% blue + 40% white
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,4),101,101,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(4,1)),101,101,255,255));

//    m.at<float>(2,0) = Core::ImageDataProvider::NoDataValue; -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,0),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,2)),0,0,0,0));

//    m.at<float>(2,1) = Core::ImageDataProvider::NoDataValue; -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,1),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,2)),0,0,0,0));

//    m.at<float>(2,2) = Core::ImageDataProvider::NoDataValue; -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,2),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,2)),0,0,0,0));

}

//*************************************************************************

}

QTEST_MAIN(Tests::ImageRendererTest)
