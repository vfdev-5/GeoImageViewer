
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
 * \brief ImageRendererTest::test verifies rendering use case with HistogramImageRenderer of 1 band non complex imagery
 * in gray mode
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
    Core::HistogramRendererConfiguration hConf;
    hConf.minValues << 0.0;
    hConf.maxValues << 400.0;
    hConf.toRGBMapping.insert(0,0);
    hConf.toRGBMapping.insert(1,0);
    hConf.toRGBMapping.insert(2,0);
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

    // Render raw data
    cv::Mat r = renderer.render(m, &hConf, true);

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

//    m.at<float>(0,3) = 300.0; -> blue | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,3),0,0,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(3,0)),0,0,255,255));

//    m.at<float>(0,4) = 400.0; -> white | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,4),255,255,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(4,0)),255,255,255,255));

//    m.at<float>(1,0) = 10.0; -> black | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,0),0,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,1)),0,0,0,255));

//    m.at<float>(1,1) = 110.0; -> 90% black + 10% red | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,1),25,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,1)),25,0,0,255));

//    m.at<float>(1,2) = 220.0; -> 80% red + 20% blue
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,2),204,0,51,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,1)),204,0,51,255));

//    m.at<float>(1,3) = 330.0;  -> 70% blue + 30% white
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,3),76,76,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(3,1)),76,76,255,255));

//    m.at<float>(1,4) = 340.0; -> 60% blue + 40% white
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,4),102,102,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(4,1)),102,102,255,255));

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
/*!
 * \brief ImageRendererTest::test verifies rendering use case with HistogramImageRenderer of 3 band non complex imagery
 * in RGB mode
 */
void ImageRendererTest::test2()
{
    // Define raw data : 3 band, nonComplex imagery
    cv::Mat m(3, 3, CV_32FC3, cv::Scalar(0));

    // cv::Mat has BGRA format
    m.at<cv::Vec3f>(0,0) = cv::Vec3f(0.0,0.0,0.0);
    m.at<cv::Vec3f>(0,1) = cv::Vec3f(100.0,50.0,20.0);
    m.at<cv::Vec3f>(0,2) = cv::Vec3f(10.0,20.0,50.0);

    m.at<cv::Vec3f>(1,0) = cv::Vec3f(20.0,50.0,100.0);
    m.at<cv::Vec3f>(1,1) = cv::Vec3f(50.0,100.0,10.0);
    m.at<cv::Vec3f>(1,2) = cv::Vec3f(200.0,150.0,220.0);

    // noDataValues :
    float nd = Core::ImageDataProvider::NoDataValue;
    m.at<cv::Vec3f>(2,0) = cv::Vec3f(nd,nd,nd);
    m.at<cv::Vec3f>(2,1) = cv::Vec3f(nd,nd,nd);
    m.at<cv::Vec3f>(2,2) = cv::Vec3f(nd,nd,nd);

    // Define renderer:
    Core::HistogramImageRenderer renderer;
    Core::HistogramRendererConfiguration hConf;
    hConf.minValues << 0.0 << 0.0 << 0.0;
    hConf.maxValues << 200.0 << 150.0 << 220.0;
    hConf.toRGBMapping.insert(0,0);
    hConf.toRGBMapping.insert(1,1);
    hConf.toRGBMapping.insert(2,2);
    hConf.mode = Core::HistogramRendererConfiguration::RGB;
    hConf.qMinValues << 0.0 << 0.0 << 0.0;
    hConf.qMaxValues << 200.0 << 150.0 << 220.0;

    // GRAY :
    hConf.transferFunctions
            << Core::HistogramRendererConfiguration::availableTransferFunctions[0]
            << Core::HistogramRendererConfiguration::availableTransferFunctions[0]
            << Core::HistogramRendererConfiguration::availableTransferFunctions[0];

    hConf.isDiscreteValues
            << false
            << false
            << false;

    // define histogram stops as :
    QGradientStops stops;
    stops << QGradientStop(0.25, Qt::black);
    stops << QGradientStop(0.5, Qt::red);
    stops << QGradientStop(0.75, Qt::blue);
    stops << QGradientStop(1.0, Qt::white);
    hConf.normHistStops << stops;


    // RGB :
    hConf.rgbTransferFunction = Core::HistogramRendererConfiguration::availableTransferFunctions[0];
    hConf.isRGBDiscreteValue = false;
    QGradientStops rStops, gStops, bStops;
    rStops << QGradientStop(0.1, Qt::black) << QGradientStop(0.9, Qt::red);
    gStops << QGradientStop(0.1, Qt::black) << QGradientStop(0.9, Qt::green);
    bStops << QGradientStop(0.1, Qt::black) << QGradientStop(0.9, Qt::blue);
    hConf.normRGBHistStops << rStops << gStops << bStops;

    // Render raw data
    cv::Mat r = renderer.render(m, &hConf, true);

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
//    m.at<cv::Vec3f>(0,0) = cv::Vec3f(0.0,0.0,0.0); -> black | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,0),0,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,0)),0,0,0,255));

    // m.at<cv::Vec3f>(0,1) = cv::Vec3f(100.0,50.0,20.0); ->
    // r : v=100.0  (0.0 [0.0 {0.1; 0.9} 1.0] 200.0) => 50% red
    // g : v=50.0  (0.0 [0.0 {0.1; 0.9} 1.0] 150.0) => 50.0 -> 0.333 => 29.1% green
    // b : v=20.0  (0.0 [0.0 {0.1; 0.9} 1.0] 220.0) => 100% black
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,1),127,74,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,0)),127,74,0,255));

    // m.at<cv::Vec3f>(0,2) = cv::Vec3f(10.0,20.0,50.0);
    // r : v=10.0  (0.0 [0.0 {0.1; 0.9} 1.0] 200.0) => 100% black
    // g : v=20.0  (0.0 [0.0 {0.1; 0.9} 1.0] 150.0) => 20.0 -> 0.1333 => 4.16% green
    // b : v=50.0  (0.0 [0.0 {0.1; 0.9} 1.0] 220.0) => 50.0 -> 0.2273 => 16% blue
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,2),0,11,41,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,0)),0,11,41,255));

    // m.at<cv::Vec3f>(1,0) = cv::Vec3f(20.0,50.0,100.0);
    // r : v=20.0  (0.0 [0.0 {0.1; 0.9} 1.0] 200.0) => 100% black
    // g : v=50.0  (0.0 [0.0 {0.1; 0.9} 1.0] 150.0) => 50.0 -> 0.3333 => 29.1% green
    // b : v=100.0  (0.0 [0.0 {0.1; 0.9} 1.0] 220.0) => 100.0 -> 0.4545 => 44.3% blue
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,0),0,74,113,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,1)),0,74,113,255));

    // m.at<cv::Vec3f>(1,1) = cv::Vec3f(50.0,100.0,10.0);
    // r : v=50.0  (0.0 [0.0 {0.1; 0.9} 1.0] 200.0) => 50.0 -> 0.25 => 18.75% red
    // g : v=100.0  (0.0 [0.0 {0.1; 0.9} 1.0] 150.0) => 100.0 -> 0.6667 => 70.8% green
    // b : v=10.0  (0.0 [0.0 {0.1; 0.9} 1.0] 220.0) => black
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,1),48,181,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,1)),48,181,0,255));

    // m.at<cv::Vec3f>(1,2) = cv::Vec3f(200.0,150.0,220.0);
    // r : v=200.0  (0.0 [0.0 {0.1; 0.9} 1.0] 200.0) => 100% red
    // g : v=150.0  (0.0 [0.0 {0.1; 0.9} 1.0] 150.0) => 100% green
    // b : v=220.0  (0.0 [0.0 {0.1; 0.9} 1.0] 220.0) => 100% blue
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,2),255,255,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,1)),255,255,255,255));

    // m.at<cv::Vec3f>(2,0) = cv::Vec3f(nd,nd,nd); -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,0),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,2)),0,0,0,0));

    // m.at<float>(2,1) = Core::ImageDataProvider::NoDataValue; -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,1),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,2)),0,0,0,0));

    // m.at<float>(2,2) = Core::ImageDataProvider::NoDataValue; -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,2),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,2)),0,0,0,0));

}

//*************************************************************************
/*!
 * \brief ImageRendererTest::test verifies rendering use case with HistogramImageRenderer of 3 band non complex imagery
 * in GRAY mode
 */
void ImageRendererTest::test3()
{
    // Define raw data : 3 band, nonComplex imagery
    cv::Mat m(3, 3, CV_32FC3, cv::Scalar(0));

    // cv::Mat has BGRA format
    m.at<cv::Vec3f>(0,0) = cv::Vec3f(0.0,0.0,0.0);
    m.at<cv::Vec3f>(0,1) = cv::Vec3f(100.0,50.0,20.0);
    m.at<cv::Vec3f>(0,2) = cv::Vec3f(10.0,20.0,50.0);

    m.at<cv::Vec3f>(1,0) = cv::Vec3f(20.0,50.0,100.0);
    m.at<cv::Vec3f>(1,1) = cv::Vec3f(50.0,100.0,10.0);
    m.at<cv::Vec3f>(1,2) = cv::Vec3f(200.0,150.0,220.0);

    // noDataValues :
    float nd = Core::ImageDataProvider::NoDataValue;
    m.at<cv::Vec3f>(2,0) = cv::Vec3f(nd,nd,nd);
    m.at<cv::Vec3f>(2,1) = cv::Vec3f(nd,nd,nd);
    m.at<cv::Vec3f>(2,2) = cv::Vec3f(nd,nd,nd);

    // Define renderer:
    Core::HistogramImageRenderer renderer;
    Core::HistogramRendererConfiguration hConf;
    hConf.minValues << 0.0 << 0.0 << 0.0;
    hConf.maxValues << 200.0 << 150.0 << 220.0;
    hConf.toRGBMapping.insert(0,0);
    hConf.toRGBMapping.insert(1,0);
    hConf.toRGBMapping.insert(2,0);
    hConf.mode = Core::HistogramRendererConfiguration::GRAY;
    hConf.qMinValues << 0.0 << 0.0 << 0.0;
    hConf.qMaxValues << 200.0 << 150.0 << 220.0;

    // GRAY :
    hConf.transferFunctions
            << Core::HistogramRendererConfiguration::availableTransferFunctions[0]
            << Core::HistogramRendererConfiguration::availableTransferFunctions[0]
            << Core::HistogramRendererConfiguration::availableTransferFunctions[0];

    hConf.isDiscreteValues
            << false
            << false
            << false;

    // define histogram stops as :
    QGradientStops stops;
    stops << QGradientStop(0.0, Qt::black);
    stops << QGradientStop(0.1, Qt::red);
    stops << QGradientStop(0.5, Qt::blue);
    stops << QGradientStop(1.0, Qt::white);
    hConf.normHistStops << stops;


    // RGB :
    hConf.rgbTransferFunction = Core::HistogramRendererConfiguration::availableTransferFunctions[0];
    hConf.isRGBDiscreteValue = false;
    QGradientStops rStops, gStops, bStops;
    rStops << QGradientStop(0.1, Qt::black) << QGradientStop(0.9, Qt::red);
    gStops << QGradientStop(0.1, Qt::black) << QGradientStop(0.9, Qt::green);
    bStops << QGradientStop(0.1, Qt::black) << QGradientStop(0.9, Qt::blue);
    hConf.normRGBHistStops << rStops << gStops << bStops;

    // Render raw data
    cv::Mat r = renderer.render(m, &hConf, true);

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

    // Test colors: Selected channel = 0
//    m.at<cv::Vec3f>(0,0) = cv::Vec3f(0.0,0.0,0.0); -> v=0.0 -> black | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,0),0,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,0)),0,0,0,255));

    // m.at<cv::Vec3f>(0,1) = cv::Vec3f(100.0,50.0,20.0); -> v=100.0 -> blue | 255
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,1),0,0,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,0)),0,0,255,255));

    // m.at<cv::Vec3f>(0,2) = cv::Vec3f(10.0,20.0,50.0); -> v=10.0 -> 50% red
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(0,2),127,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,0)),127,0,0,255));

    // m.at<cv::Vec3f>(1,0) = cv::Vec3f(20.0,50.0,100.0); -> v=20.0 -> 100% red
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,0),255,0,0,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,1)),255,0,0,255));

    // m.at<cv::Vec3f>(1,1) = cv::Vec3f(50.0,100.0,10.0); -> v=50.0 -> 63% red + 37% blue
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,1),159,0,96,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,1)),159,0,96,255));

    // m.at<cv::Vec3f>(1,2) = cv::Vec3f(200.0,150.0,220.0); -> v=200.0 -> white
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(1,2),255,255,255,255));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,1)),255,255,255,255));

    // m.at<cv::Vec3f>(2,0) = cv::Vec3f(nd,nd,nd); -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,0),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(0,2)),0,0,0,0));

    // m.at<float>(2,1) = Core::ImageDataProvider::NoDataValue; -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,1),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(1,2)),0,0,0,0));

    // m.at<float>(2,2) = Core::ImageDataProvider::NoDataValue; -> black | 0
    QVERIFY(testBGRAColor(r.at<cv::Vec4b>(2,2),0,0,0,0));
    QVERIFY(testRGBAColor2(QColor::fromRgba(im.pixel(2,2)),0,0,0,0));

}

//*************************************************************************

}

QTEST_MAIN(Tests::ImageRendererTest)
