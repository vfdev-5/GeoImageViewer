
// Qt
#include <QFileInfo>
#include <QThreadPool>


// Project
#include "ImageWriter.h"
#include "ImageDataProvider.h"
#include "GeoImageLayer.h"

namespace Core
{

//******************************************************************************
/*!
  \class ImageWriter
  \brief
 */
//******************************************************************************

ImageWriter::ImageWriter(QObject *parent) :
    QObject(parent),
    _isWorking(false),
    _isAsyncTask(true),
    _task(0)
{
    int count = GetGDALDriverManager()->GetDriverCount();
    if (count == 0)
        GDALAllRegister();
}

//******************************************************************************

ImageWriter::~ImageWriter()
{
    int count = GetGDALDriverManager()->GetDriverCount();
    if (count > 0)
        GDALDestroyDriverManager();
}

//******************************************************************************

bool ImageWriter::write(const QString &outputfilename, const ImageDataProvider *data, const GeoImageLayer * dataInfo)
{
    if (QFileInfo(outputfilename).exists())
    {
        if (!QFile(outputfilename).remove())
        {
            SD_ERR(tr("Failed to remove existing output file : %1").arg(outputfilename));
            return false;
        }
    }

    if (!_task)
        _task = new WriteImageTask(this);


    _task->setOutputFile(outputfilename);
    _task->setDataProvider(data);
    _task->setDataInfo(dataInfo);
    _isAsyncTask = false;
    _task->run();
    return QFileInfo(outputfilename).exists();
}

//******************************************************************************

bool ImageWriter::writeInBackground(const QString &outputfilename, const ImageDataProvider *data, const GeoImageLayer *dataInfo)
{
    if (!removeFile(outputfilename))
    {
        SD_ERR(tr("Failed to remove existing output file : %1").arg(outputfilename));
        return false;
    }

    if (!_task)
        _task = new WriteImageTask(this);

    _task->setOutputFile(outputfilename);
    _task->setDataProvider(data);
    _task->setDataInfo(dataInfo);
    _isAsyncTask = true;

    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    pool->waitForDone();
    // Only one thread is possible due to GDAL reader (e.g. TIFF)
    _isWorking=true;
    pool->start(_task);
    return true;
}

//******************************************************************************

bool ImageWriter::writeInBackground(const QString &outputfilename, const QImage * data, const GeoImageLayer *dataInfo)
{
    if (!removeFile(outputfilename))
    {
        SD_ERR(tr("Failed to remove existing output file : %1").arg(outputfilename));
        return false;
    }

    if (!_task)
        _task = new WriteImageTask(this);

    _task->setOutputFile(outputfilename);
    _task->setDataProvider(data);
    _task->setDataInfo(dataInfo);
    _isAsyncTask = true;

    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    pool->waitForDone();
    // Only one thread is possible due to GDAL reader (e.g. TIFF)
    _isWorking=true;
    pool->start(_task);
    return true;
}

//******************************************************************************

void ImageWriter::cancel()
{
    _task->setOutputFile(QString());
    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    pool->waitForDone();
    _isWorking=false;
}

//******************************************************************************

bool ImageWriter::removeFile(const QString &filename)
{
    if (QFileInfo(filename).exists())
    {
        if (!QFile(filename).remove())
        {
            return false;
        }
    }
    return true;
}

//******************************************************************************

void ImageWriter::taskFinished(bool ok)
{
    _isWorking=false;
    if (_isAsyncTask)
        emit imageWriteFinished(ok);
}

//******************************************************************************
//******************************************************************************

#define Cancel() \
    if (_canceled) { \
        _imageWriter->taskFinished(false); \
        return; \
    }

void WriteImageTask::run()
{

    _canceled=false;

    if (_filename.isEmpty() ||
            (_dataProvider == 0 && _sourceImage == 0) ||
            _dataInfo == 0)
    {
        SD_TRACE("WriteImageTask::run : filename is empty or no data providers or no data info");
        _imageWriter->taskFinished(false);
        return;
    }


    bool res = false;
    Cancel();

    // set Geo info
    QString projStr = _dataInfo->getProjectionRef();
    QVector<double> geoTransform = _dataInfo->getGeoTransform();
    double nodatavalue = ImageDataProvider::NoDataValue;
    QList< QPair<QString, QString> > metadata = _dataInfo->getMetadata();

    Cancel();
    _imageWriter->writeProgressValueChanged(10);


    cv::Mat data;
    QImage im;
    if (_dataProvider)
    {
        // get data to write :
        data = _dataProvider->getImageData();
    }
    else if (_sourceImage)
    {
        im = _sourceImage->copy();
        data = Core::fromQImage(im);
//        Core::printMat(data, "data");
    }

    Cancel();
    _imageWriter->writeProgressValueChanged(50);
    res = Core::writeToFile(_filename, data,
                                 projStr, geoTransform,
                                 nodatavalue, metadata);

    _imageWriter->writeProgressValueChanged(100);
    _imageWriter->taskFinished(res);

    // reset conf:
    _dataProvider = 0;
    _sourceImage = 0;
    _dataInfo = 0;

}

//******************************************************************************

}
