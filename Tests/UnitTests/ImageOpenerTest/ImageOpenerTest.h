#ifndef IMAGEOPENERTEST_H
#define IMAGEOPENERTEST_H

// Qt
#include <QObject>
#include <QtTest>

// Project
#include "Core/ImageOpener.h"
#include "Core/ImageDataProvider.h"

namespace Tests
{

//*************************************************************************

class ImageOpenerTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test();
    void test2();
    void cleanupTestCase();

protected:
    void onImageOpened(Core::ImageDataProvider*);

private:
    Core::ImageOpener * _imageOpener;
    QUrl _url;
    QUrl _url2;
    bool imageOpened;

};



//*************************************************************************

} 

#endif // IMAGEOPENERTEST_H
