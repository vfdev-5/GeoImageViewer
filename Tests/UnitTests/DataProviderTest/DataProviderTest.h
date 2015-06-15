#ifndef DATAPROVIDERTEST_H
#define DATAPROVIDERTEST_H

// Qt
#include <QObject>
#include <QtTest>

// Opencv
#include <opencv2/core/core.hpp>

// Project
#include "Core/ImageDataProvider.h"
#include "Core/FloatingDataProvider.h"

namespace Tests
{

//*************************************************************************

class DataProviderTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test_computeMask();
    void test_GDALDataProvider();
    void test_GDALDataProvider2();
    void test_FloatingDataProvider();
    void test_FloatingDataProvider2();
    void test_FloatingDataProvider3();
//    void test_FloatingDataProvider4();
    void cleanupTestCase();

private:
    Core::GDALDataProvider * _provider;

    QStringList _testFiles;
    QList<cv::Mat> _testMatrices;
    QString _projectionStr;
    QVector<double> _geoTransform;
    double _noDataValue;
    QList< QPair<QString,QString> > _metadata;
    QPolygonF _geoExtent;

};



//*************************************************************************

} 

#endif // DATAPROVIDERTEST_H
