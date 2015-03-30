#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H


// Qt
#include <QObject>
#include <QRunnable>

// Project
#include "LibExport.h"
#include "LayerUtils.h"

namespace Core
{

class GeoImageLayer;
class ImageDataProvider;
class WriteImageTask;

//******************************************************************************

class GIV_DLL_EXPORT ImageWriter : public QObject
{
    Q_OBJECT
    friend class WriteImageTask;
public:
    explicit ImageWriter(QObject *parent = 0);
    virtual ~ImageWriter();

    bool write(const QString & outputfilename, const Core::ImageDataProvider * data, const GeoImageLayer * dataInfo);
    bool writeInBackground(const QString & outputfilename, const Core::ImageDataProvider * data, const GeoImageLayer * dataInfo);
    void cancel();
    bool isWorking()
    { return _isWorking; }

signals:
    void imageWriteFinished(bool ok);
    void writeProgressValueChanged(int);

private:

    void taskFinished(bool);
    WriteImageTask * _task;
    bool _isAsyncTask;
    bool _isWorking;
};

//******************************************************************************

class WriteImageTask : public QObject, public QRunnable
{
    Q_OBJECT
public:

    WriteImageTask(ImageWriter * parent) :
        QObject(parent),
        _imageWriter(parent),
        _canceled(false),
        _dataProvider(0)
    {
        setAutoDelete(false);
    }

    void setOutputFile(const QString & filename)
    {
        _canceled = true;
        _filename = filename;
    }
    virtual void run();

    void setDataProvider(const ImageDataProvider * p)
    { _dataProvider = p; }

    void setDataInfo(const GeoImageLayer * info)
    { _dataInfo = info; }

protected:

    bool _canceled;
    QString _filename;
    ImageWriter * _imageWriter;
    const ImageDataProvider * _dataProvider;
    const GeoImageLayer * _dataInfo;
};

//******************************************************************************

}

#endif // IMAGEWRITER_H
