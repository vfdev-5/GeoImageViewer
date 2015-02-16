#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

// Qt
#include <QObject>
#include <QString>
#include <QHash>
#include <QRunnable>

// Project
#include "LayerTools.h"

namespace Core
{

//******************************************************************************

class Image;
class ImageLayer;
class LoadImageTask;

class ImageManager : public QObject
{
    Q_OBJECT
    friend class LoadImageTask;

public:

    static ImageManager * get()
    {
        if (!_instance) { _instance = new ImageManager(); }
        return _instance;
    }

    static void destroy()
    {
        if (_instance) {
            delete _instance;
        }
    }

// WHY not to use simply get() ??? Singleton is not be destroyed when the app is destroyed ?
//    static ImageManager * init(QObject * parent)
//    {
//        if (_instance) return 0;
//        _instance = new ImageManager(parent);
//        return _instance;
//    }

    Image * loadImageConcurrent(const QString & filepath)
    { return loadImage(filepath); }

    void loadImageInBackground(const QString &fileToOpen, const QString & originalFilePath);

    ImageLayer * getImageLayer(Image * image)
    { return _map[image]; }

public slots:
    void onLoadCanceled();

signals:
    void loadFinished(Core::Image*);
    void loadProgressValueChanged(int);

protected:

    Image * loadImage(const QString & filepath);

    ImageManager(QObject *parent = 0);
    ~ImageManager();

    static ImageManager * _instance;

    QHash<Image*, ImageLayer*> _map;

    LoadImageTask * _loadImageTask;

};

//******************************************************************************

class LoadImageTask : public QObject, public QRunnable {
    Q_OBJECT

public:

    LoadImageTask(ImageManager * parent) :
        QObject(parent),
        _manager(parent),
        _canceled(false),
        _reporter(new ProgressReporter(this)),
        _image(0)
    {
        setAutoDelete(false);
        connect(_reporter, SIGNAL(progressValueChanged(int)),
                _manager, SIGNAL(loadProgressValueChanged(int)));
    }

//    void setImagePath(const QString & path);
    void setImage(Image * image);

protected:

    void run();
    bool _canceled;
//    QString _path;
    Image * _image;
    ImageManager * _manager;
    ProgressReporter *_reporter;

};

//******************************************************************************

}

#endif // IMAGEMANAGER_H
