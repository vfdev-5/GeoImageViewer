#ifndef COLORTEST_H
#define COLORTEST_H

// Qt
#include <QObject>
#include <QtTest>

// Project

namespace Tests
{

//*************************************************************************

class ColorTest : public QObject
{
    Q_OBJECT
private slots:
    void test();

private:

};

//*************************************************************************

} 

#endif // COLORTEST_H
