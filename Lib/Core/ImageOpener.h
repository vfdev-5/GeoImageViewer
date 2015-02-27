#ifndef IMAGEOPENER_H
#define IMAGEOPENER_H


// Qt
#include <QObject>
#include <QRunnable>
#include <QUrl>
#include <QRect>
#include <QRectF>
#include <QPolygonF>

// Project
#include "LibExport.h"
#include "LayerUtils.h"

namespace Core
{

class OpenImageTask;
class ImageDataProvider;

//******************************************************************************

class GIV_DLL_EXPORT ImageOpener : public QObject
{
    Q_OBJECT
    friend class OpenImageTask;
    friend class OpenImageFileTask;
public:

    explicit ImageOpener(QObject *parent = 0);
    virtual ~ImageOpener();
    bool openImageInBackground(const QUrl & url);
    void cancel();

    ImageDataProvider * openImage(const QUrl & url);

signals:
    void imageOpened(Core::ImageDataProvider *);
    void openProgressValueChanged(int);

protected:
    QString handleLocalFile(const QUrl & url);

private:
    void taskFinished(Core::ImageDataProvider *);
    OpenImageTask * _task;
    bool _isAsyncTask;

};

//******************************************************************************

class OpenImageTask : public QObject, public QRunnable
{
    Q_OBJECT

    PTR_PROPERTY_ACCESSORS(ImageDataProvider, dataProvider, getDataProvider, setDataProvider)

public:

    OpenImageTask(ImageOpener * parent) :
        QObject(parent),
        _imageOpener(parent),
        _canceled(false),
        _reporter(new ProgressReporter(this)),
        _dataProvider(0)
    {
        setAutoDelete(false);
        connect(_reporter, SIGNAL(progressValueChanged(int)),
                _imageOpener, SIGNAL(openProgressValueChanged(int)));
    }

    void setImage(const QString & path)
    {
        _canceled = true;
        _path = path;
    }
    virtual void run() = 0;

protected:

    bool _canceled;
    QString _path;
    ImageOpener * _imageOpener;
    ProgressReporter *_reporter;

};

//******************************************************************************

class OpenImageFileTask : public OpenImageTask
{
    Q_OBJECT
public:
    OpenImageFileTask(ImageOpener * parent);

    virtual void run();

protected:

};

//******************************************************************************

}

#endif // IMAGEOPENER_H
