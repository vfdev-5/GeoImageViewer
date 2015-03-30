#ifndef DATAPROVIDERTEST_H
#define DATAPROVIDERTEST_H

// Qt
#include <QObject>
#include <QtTest>

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
    void test();
    void test2();
    void test2_1();
    void test3();
    void cleanupTestCase();

private:
    Core::GDALDataProvider * _provider;
};



//*************************************************************************

} 

#endif // DATAPROVIDERTEST_H
