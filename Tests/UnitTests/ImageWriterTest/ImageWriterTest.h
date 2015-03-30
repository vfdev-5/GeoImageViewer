#ifndef IMAGEWRITERTEST_H
#define IMAGEWRITERTEST_H

// Qt
#include <QObject>
#include <QtTest>

// Project
#include "Core/ImageWriter.h"
#include "Core/FloatingDataProvider.h"
#include "Core/GeoImageLayer.h"

namespace Tests
{

//*************************************************************************

class ImageWriterTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test();
    void test2();
    void cleanupTestCase();

protected:
    void onImageWriteFinished(bool ok);

private:
    Core::ImageWriter * _imageWriter;
    Core::FloatingDataProvider * _provider;
    Core::GeoImageLayer * _geoInfo;
    bool writeFinished;
};



//*************************************************************************

} 

#endif // IMAGEWRITERTEST_H
