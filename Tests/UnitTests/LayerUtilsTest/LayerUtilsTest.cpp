
// Qt
#include <QDir>
#include <QFileInfo>
#include <QLabel>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Tests
#include "../../Common.h"
#include "LayerUtilsTest.h"
#include "Core/LayerUtils.h"
#include "Core/ImageDataProvider.h"

namespace Tests
{

int WIDTH  = 2000;
int HEIGHT = 2000;
int DEPTH  = CV_16U;
//int DEPTH2  = CV_16U;
cv::Mat TEST_MATRIX;
QString PROJECTION_STR;
QVector<double> GEO_TRANSFORM;
double NO_DATA_VALUE=(1<<16) - 1;
QList< QPair<QString,QString> > METADATA;
QPolygonF GEO_EXTENT;

//*************************************************************************

void LayerUtilsTest::initTestCase()
{
    // Create temporary dir
    QDir d = QDir::currentPath() + "/Temp_tests_123";
    SD_TRACE("Temporary dir : " + d.absolutePath());
    if (!d.exists())
    {
        d.mkdir(d.absolutePath());
    }
    QDir::addSearchPath("Input", d.absolutePath());


    // Register GDAL drivers
    GDALAllRegister();

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

    PROJECTION_STR = Core::getProjectionStrFromGeoCS();

    GEO_TRANSFORM.resize(6);
    GEO_TRANSFORM[0] = 1.358847; // Origin X
    GEO_TRANSFORM[3] = 43.575298;  // Origin Y
    GEO_TRANSFORM[2] = GEO_TRANSFORM[4] = 0.0;
    GEO_TRANSFORM[1] = 0.0001; // Step X
    GEO_TRANSFORM[5] = -0.0001;// Step Y
    METADATA <<  QPair<QString,QString>("My_MD_1","THIS IS A TEST IMAGE");
    METADATA <<  QPair<QString,QString>("My_MD_VERSION","0.0");
    METADATA <<  QPair<QString,QString>("My_MD_GEO","Somewhere");
    METADATA <<  QPair<QString,QString>("My_MD_Satellite","NA");
    // (0,0) -> (w-1,0) -> (w-1,h-1) -> (0,h-1)
    GEO_EXTENT << QPointF(GEO_TRANSFORM[0], GEO_TRANSFORM[3]);
    GEO_EXTENT << QPointF(GEO_TRANSFORM[0]+GEO_TRANSFORM[1]*(WIDTH-1), GEO_TRANSFORM[3]);
    GEO_EXTENT << QPointF(GEO_TRANSFORM[0]+GEO_TRANSFORM[1]*(WIDTH-1), GEO_TRANSFORM[3]+GEO_TRANSFORM[5]*(HEIGHT-1));
    GEO_EXTENT << QPointF(GEO_TRANSFORM[0], GEO_TRANSFORM[3]+GEO_TRANSFORM[5]*(HEIGHT-1));
}

//*************************************************************************

void LayerUtilsTest::test_GeoComputationMethods()
{
    QVERIFY(Core::isGeoProjection(PROJECTION_STR));
    QVERIFY(!Core::isGeoProjection("PROJECTION_STR"));

    QVERIFY(Core::compareProjections(PROJECTION_STR, PROJECTION_STR));
    QVERIFY(!Core::compareProjections(PROJECTION_STR, "PROJECTION_STR"));

    QVERIFY(Core::isEqual(TEST_MATRIX,TEST_MATRIX));
    QVERIFY(!Core::isEqual(TEST_MATRIX,TEST_MATRIX+2.0));

    QRect pixelExtent = QRect(0,0,WIDTH,HEIGHT);
    QVector<double> gt = Core::computeGeoTransform(GEO_EXTENT, pixelExtent);
    QVERIFY(compareVectors(gt, GEO_TRANSFORM));

}

//*************************************************************************

void LayerUtilsTest::test_OpencvOperations()
{
    try
    {
        cv::Mat m1(100,100,CV_16UC(5));
        m1.setTo(1);
        cv::Mat m2(100,100,CV_16UC(5));
        m2.setTo(1);
        m2 = m2.mul(5);
        m1 = m1 + m2;
        m1 = m1 * 2 + m2;
        QVERIFY(true);
    }
    catch (const cv::Exception & e)
    {
        QVERIFY(false);
    }
}

//*************************************************************************

//void LayerUtilsTest::test_VectorizeAsPolygons()
//{
//    cv::Mat m(100, 100, CV_8U, cv::Scalar(0));
//    // box: (10,10) -> (10,19) -> (19,19) -> (19,10) -> (10,10)
//    cv::Mat m1(10, 10, CV_8U, cv::Scalar(255));
//    m1.copyTo(m(cv::Rect(10, 10, 10, 10)));
//    // rectangle: (50,5) -> (50,19) -> (84,19) -> (84,5) -> (50,5)
//    cv::Mat m2(15, 35, CV_8U, cv::Scalar(255));
//    m2.copyTo(m(cv::Rect(50, 5, 35, 15)));
//    // cross: (25,40) ->
//    cv::Mat m3(20, 7, CV_8U, cv::Scalar(255));
//    m3.copyTo(m(cv::Rect(25, 40, 7, 20)));
//    m3 = cv::Mat(7, 20, CV_8U, cv::Scalar(255));
//    m3.copyTo(m(cv::Rect(20, 45, 20, 7)));

////    Core::displayMat(m, true, "m");

//    QVector<QPolygonF> contours = Core::vectorizeAsPolygons(m);

//    QVERIFY(contours.size() == 3);

//    foreach (QPolygonF contour, contours)
//    {

//        if (contour[0] == QPointF(10, 10))
//        {
//            QVERIFY(contour.size() == 5);
//            QVERIFY(contour[1] == QPointF(10,19));
//            QVERIFY(contour[1] == QPointF(19,19));
//            QVERIFY(contour[1] == QPointF(19,10));
//        }
//        else if (contour[0] == QPointF(50, 5))
//        {
//            QVERIFY(contour.size() == 5);
//            QVERIFY(contour[1] == QPointF(50,19));
//            QVERIFY(contour[1] == QPointF(84,19));
//            QVERIFY(contour[1] == QPointF(84,5));
//        }
//        else if (contour[0] == QPointF(25, 40))
//        {
//            QVERIFY(contour.size() >= 13);
////            QVERIFY(contour.indexOf(QPointF() >= 0);
//        }
//    }
//    QVERIFY(true);
//}

//*************************************************************************

cv::Mat computeMaskND(const cv::Mat & data, float noDataValue)
{
    cv::Mat m = data != noDataValue;
    cv::Mat out;
    m.convertTo(out, data.depth(), 1.0/255.0);
    return out;
}

bool checkMatrices(const cv::Mat & m1, const cv::Mat & m2, float nodatavalue)
{
    cv::Mat mask = Core::ImageDataProvider::computeMask(m1);
    cv::Mat mask2 = Core::ImageDataProvider::computeMask(m2, nodatavalue);
    if (!Core::isEqual(mask,mask2))
        return false;

    cv::Mat maskND, mask2ND;
    maskND = computeMaskND(m1, Core::ImageDataProvider::NoDataValue);
    mask2ND = computeMaskND(m2, nodatavalue);

    cv::Mat mm1 = m1.mul(maskND);
    cv::Mat mm2 = m2.mul(mask2ND);

    if (!Core::isEqual(mm1,mm2))
        return false;

    return true;
}

void LayerUtilsTest::test_FileReadWriteMethods()
{

    QString out = QFileInfo("Input:").absoluteFilePath() + "/test_image0.tif";
    bool res = Core::writeToFile(out, TEST_MATRIX,
                                 PROJECTION_STR, GEO_TRANSFORM,
                                 NO_DATA_VALUE, METADATA);
    QVERIFY(res);
    QVERIFY(QFile(out).exists());


    Core::GDALDataProvider * provider = new Core::GDALDataProvider();
    QVERIFY(provider->setup(out));
    cv::Mat m = provider->getImageData();
    cv::Mat m2;
    TEST_MATRIX.convertTo(m2, m.depth());
    //    Core::displayMat(m, true, "m");
    //    Core::displayMat(m2, true, "m2");
    QVERIFY(checkMatrices(m,m2,NO_DATA_VALUE));
    // Check geo info :
    QVERIFY( Core::compareProjections(provider->fetchProjectionRef(), PROJECTION_STR) );
    QVERIFY( comparePolygons(provider->fetchGeoExtent(), GEO_EXTENT) );
    QVERIFY( compareVectors(provider->fetchGeoTransform(), GEO_TRANSFORM) );
    QVERIFY( provider->getPixelExtent() == QRect(0,0,WIDTH,HEIGHT));
    delete provider;

    QVERIFY(QFile(out).remove());
}

//*************************************************************************

void LayerUtilsTest::test_Mat2QImage()
{
    cv::Mat rgbaSource(500, 500, CV_8UC4, cv::Scalar(0,0,0,255));
    int offsetX = 1;
    int offsetY = 0;
    for (int i=offsetY;i<offsetY+50;i++)
    {
        for (int j=offsetX;j<offsetX+100;j++)
        {
            rgbaSource.at<cv::Vec4b>(i,j) = cv::Vec4b(0,0,112,255);
        }
    }

    //    Core::printMat(rgbaSource, "SRC");

    QImage im = Core::fromMat(rgbaSource).copy();

    //    std::cout << "Print QImage :"<< std::endl;
    //    for (int i=0;i<10;i++)
    //    {
    //        for (int j=0;j<10;j++)
    //        {
    //            QRgb rgba = im.pixel(j,i);
    //            std::cout << "(";
    //            std::cout << qRed(rgba) << " "
    //                      << qGreen(rgba) << " "
    //                      << qBlue(rgba) << " "
    //                      << qAlpha(rgba);
    //            std::cout << ")";
    //        }
    //        std::cout << std::endl;
    //    }
    //    std::cout << "------"<< std::endl;

    //    QLabel * label = new QLabel();
    //    label->setPixmap(QPixmap::fromImage(im));
    //    label->show();

    for (int i=0;i<rgbaSource.rows;i++)
    {
        for (int j=0;j<rgbaSource.cols;j++)
        {
            QRgb rgba = im.pixel(j,i);
            QVERIFY(testRGBAColor(rgbaSource.at<cv::Vec4b>(i,j),
                                  qRed(rgba),
                                  qGreen(rgba),
                                  qBlue(rgba),
                                  qAlpha(rgba))
                    );
        }
    }

    cv::Mat rgbaDst = Core::fromQImage(im);
    //    Core::displayMat(rgbaSource, true, "DST");

    QVERIFY(Core::isEqual(rgbaDst, rgbaSource));

    //    label->hide();
    //    delete label;

}

//*************************************************************************

void LayerUtilsTest::test_displayMat()
{
    cv::Mat src(500, 500, CV_8UC3, cv::Scalar(127,127,127));
    int offsetX = 1;
    int offsetY = 0;
    for (int i=offsetY;i<offsetY+50;i++)
    {
        for (int j=offsetX;j<offsetX+100;j++)
        {
            src.at<cv::Vec4b>(i,j) = cv::Vec4b(0,0,255);
        }
    }

    cv::Mat dst = Core::displayMat(src, false, "SRC", false);
    cv::cvtColor(dst, dst, cv::COLOR_RGB2BGR);
    //    Core::printMat(dst, "DST");
    cv::destroyAllWindows();

    QVERIFY(Core::isEqual(dst, src));

}

//*************************************************************************

void LayerUtilsTest::test_computeMask()
{
    float noDataValue = -1;
    cv::Mat inputImage(10, 10, CV_32FC3);
    inputImage.setTo(noDataValue);

    for (int i=4; i<7; i++)
    {
        for (int j=3; j<8; j++)
        {
            inputImage.at<cv::Vec3f>(i, j) = cv::Vec3f(i*j + j, i, j);
        }
    }
    inputImage.at<cv::Vec3f>(0, 0) = cv::Vec3f(-1, 0, 0);
    inputImage.at<cv::Vec3f>(0, 1) = cv::Vec3f(-1, -1, 0);
    inputImage.at<cv::Vec3f>(0, 2) = cv::Vec3f(0, -1, -1);
    inputImage.at<cv::Vec3f>(0, 3) = cv::Vec3f(0, 0, -1);
    inputImage.at<cv::Vec3f>(0, 4) = cv::Vec3f(-1, 0, -1);

    cv::Mat unmask, mask = Core::computeMask(inputImage, noDataValue, &unmask);

    // Check result
    QVERIFY(mask.at<cv::Vec3f>(0,0)[0] == 0 && mask.at<cv::Vec3f>(0,0)[1] == 1 && mask.at<cv::Vec3f>(0,0)[2] == 1);
    QVERIFY(mask.at<cv::Vec3f>(0,1)[0] == 0 && mask.at<cv::Vec3f>(0,1)[1] == 0 && mask.at<cv::Vec3f>(0,1)[2] == 1);
    QVERIFY(mask.at<cv::Vec3f>(0,2)[0] == 1 && mask.at<cv::Vec3f>(0,2)[1] == 0 && mask.at<cv::Vec3f>(0,2)[2] == 0);
    QVERIFY(mask.at<cv::Vec3f>(0,3)[0] == 1 && mask.at<cv::Vec3f>(0,3)[1] == 1 && mask.at<cv::Vec3f>(0,3)[2] == 0);
    QVERIFY(mask.at<cv::Vec3f>(0,4)[0] == 0 && mask.at<cv::Vec3f>(0,4)[1] == 1 && mask.at<cv::Vec3f>(0,4)[2] == 0);
    QVERIFY(unmask.at<cv::Vec3f>(0,0)[0] == 1 && unmask.at<cv::Vec3f>(0,0)[1] == 0 && unmask.at<cv::Vec3f>(0,0)[2] == 0);
    QVERIFY(unmask.at<cv::Vec3f>(0,1)[0] == 1 && unmask.at<cv::Vec3f>(0,1)[1] == 1 && unmask.at<cv::Vec3f>(0,1)[2] == 0);
    QVERIFY(unmask.at<cv::Vec3f>(0,2)[0] == 0 && unmask.at<cv::Vec3f>(0,2)[1] == 1 && unmask.at<cv::Vec3f>(0,2)[2] == 1);
    QVERIFY(unmask.at<cv::Vec3f>(0,3)[0] == 0 && unmask.at<cv::Vec3f>(0,3)[1] == 0 && unmask.at<cv::Vec3f>(0,3)[2] == 1);
    QVERIFY(unmask.at<cv::Vec3f>(0,4)[0] == 1 && unmask.at<cv::Vec3f>(0,4)[1] == 0 && unmask.at<cv::Vec3f>(0,4)[2] == 1);

    for (int i=0; i<10; i++)
    {
        for (int j=0; j<10; j++)
        {
            if (i==0 && j >= 0 && j < 5)
                continue;

            if (i >=4 && i<7 && j>=3 && j<8)
            {
                QVERIFY(mask.at<cv::Vec3f>(i,j) == cv::Vec3f(1, 1, 1));
                QVERIFY(unmask.at<cv::Vec3f>(i,j) == cv::Vec3f(0, 0, 0));
            }
            else
            {
                QVERIFY(mask.at<cv::Vec3f>(i,j) != cv::Vec3f(1, 1, 1));
                QVERIFY(unmask.at<cv::Vec3f>(i,j) != cv::Vec3f(0, 0, 0));
            }
        }
    }
}

//*************************************************************************

void LayerUtilsTest::test_joinContours()
{
    // Test cross overlapping
    {
        // Init
        QVector< QPolygon > contours(2);
        QPolygon & c1 = contours[0];
        QPolygon & c2 = contours[1];

        c1 << QPoint(-3, 2) << QPoint(-1, 2) << QPoint(1, 2)
           << QPoint(3, 2) << QPoint(3, -2) << QPoint(1, -2)
           << QPoint(-1, -2) << QPoint(-3, -2) << QPoint(-3, 2);
        c1.translate(5, 5);

        c2 << QPoint(-2, -2) << QPoint(-2, -1) << QPoint(-2, 1)
           << QPoint(-2, 2) << QPoint(2, 2) << QPoint(2, 1)
           << QPoint(2, -1) << QPoint(2, -2) << QPoint(-2, -2);
        c2.translate(5, 5);

        std::vector<std::vector<cv::Point> > cc;
        cv::Mat v1(10, 10, CV_8U, cv::Scalar::all(0));
        Core::toStdContours(contours, cc);
        cv::drawContours(v1, cc, -1, cv::Scalar::all(1.0), CV_FILLED);
        Core::printMat(v1, "Cross", 10);

        int count = Core::joinOvrlContours(contours);
        QVERIFY(count == 1);

        cv::Mat v2(10, 10, CV_8U, cv::Scalar::all(0));
        Core::toStdContours(contours, cc);
        cv::drawContours(v2, cc, -1, cv::Scalar::all(1.0), CV_FILLED);
        Core::printMat(v2, "Cross", 10);



    }

}

//*************************************************************************

void LayerUtilsTest::cleanupTestCase()
{

    // remove temporary directory:
    QDir d("Input:");
    QVERIFY(d.removeRecursively());


    GDALDestroyDriverManager();
}

//*************************************************************************

}

QTEST_MAIN(Tests::LayerUtilsTest)
