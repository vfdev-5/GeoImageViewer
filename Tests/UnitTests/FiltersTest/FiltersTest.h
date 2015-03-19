#ifndef FILTERSTEST_H
#define FILTERSTEST_H

// Qt
#include <QObject>
#include <QtTest>

// Project

namespace Tests
{

//*************************************************************************

class FiltersTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test_exception();
    void test2();
    void test3();
    void cleanupTestCase();

private:
};



//*************************************************************************

} 

#endif // FILTERSTEST_H
