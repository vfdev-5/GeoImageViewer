#ifndef IMAGEWRITERTEST_H
#define IMAGEWRITERTEST_H

// Qt
#include <QObject>
#include <QtTest>

// Project
#include "Core/ImageWriter.h"
#include "Core/ImageDataProvider.h"

namespace Tests
{

//*************************************************************************

class ImageWriterTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test();
    void cleanupTestCase();

protected:
    void onImageWriteFinished(bool ok);

private:
    Core::ImageWriter * _imageWriter;
    Core::ImageDataProvider * _provider;
};



//*************************************************************************

} 

#endif // IMAGEOPENERTEST_H
