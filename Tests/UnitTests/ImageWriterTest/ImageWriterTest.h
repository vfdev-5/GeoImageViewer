#ifndef IMAGEWRITERTEST_H
#define IMAGEWRITERTEST_H

// Qt
#include <QObject>
#include <QtTest>
#include <QImage>

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
    void test_write();
    void test_writeInBackground();
    void test_writeInBackground_cancel();
    void test_scribble_writeInBackground();
    void cleanupTestCase();

protected:
    void onImageWriteFinished(bool ok);
    void onImageWriteCanceled(bool ok);
    void onImageWriteFinished2(bool ok);

private:
    Core::ImageWriter * _imageWriter;
    Core::FloatingDataProvider * _provider;
    Core::GeoImageLayer * _geoInfo;
    bool writeFinished;
    QString _outFilename;
    QImage _scribble;
};



//*************************************************************************

} 

#endif // IMAGEWRITERTEST_H
