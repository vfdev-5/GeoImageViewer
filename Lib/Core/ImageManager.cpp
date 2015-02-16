
// Qt
#include <QFileInfo>
#include <QThreadPool>

// GDAL
#include <gdal_priv.h>

// Project
#include "Image.h"
#include "ImageManager.h"
#include "ImageLayer.h"

namespace Core
{

ImageManager * ImageManager::_instance = 0;

//******************************************************************************

ImageManager::ImageManager(QObject *parent) :
    QObject(parent),
    _loadImageTask(new LoadImageTask(this))
{
    GDALAllRegister();
}

//******************************************************************************

ImageManager::~ImageManager()
{
    onLoadCanceled();
    // Remove images
    if (!_map.isEmpty())
    {
        foreach (Image * image, _map.keys())
        {
            ImageLayer * layer = _map.value(image);
            delete image;
            delete layer;
        }
    }
    GDALDestroyDriverManager();
}

//******************************************************************************

Image * ImageManager::loadImage(const QString & filepath)
{
    ImageLayer * layer = new ImageLayer();
    if (!layer->setup(filepath))
    {
        delete layer;
        return 0;
    }
    Image * image = new Image();
    image->setFilePath(filepath);
    image->setImageName(QFileInfo(filepath).baseName());

    _map.insert(image, layer);

    return image;
}

//******************************************************************************

void ImageManager::loadImageInBackground(const QString &fileToOpen, const QString &originalFilePath)
{
    // Create image & set initial info
    Image * image = new Image();
    image->setFilePath(originalFilePath);
    image->setFileToOpen(fileToOpen);
    image->setImageName(QFileInfo(originalFilePath).baseName());

    // Cancel previous work & load new image:
//    _loadImageTask->setImagePath(fileToOpen);
    _loadImageTask->setImage(image);

    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    pool->waitForDone();
    // Only one thread is possible due to GDAL reader (e.g. TIFF)
    pool->start(_loadImageTask);

}

//******************************************************************************

void ImageManager::onLoadCanceled()
{
//    _loadImageTask->setImagePath("");
    _loadImageTask->setImage(0);
    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    pool->waitForDone();

}

//******************************************************************************
//******************************************************************************


#define ClearData() \
if (layer) delete layer; \
if (_image){ delete _image; _image=0; }

#define Cancel() \
if (_canceled) { \
    ClearData() \
    return; \
}

void setupImageInfo(ImageLayer * layer, Image * image)
{
    GDALDataset * dataset = layer->getDataset();

    image->setNbBands(dataset->GetRasterCount());
    image->setPixelExtent(layer->getPixelExtent());
    image->setGeoExtent(computeGeoExtent(dataset));
    image->setGeoBBox(image->getGeoExtent().boundingRect());
    image->setProjectionRef(QString(dataset->GetProjectionRef()));

    GDALDataType datatype = dataset->GetRasterBand(1)->GetRasterDataType();
    image->setDepthInBytes(GDALGetDataTypeSize(datatype)/8);
    image->setIsComplex(GDALDataTypeIsComplex(datatype) == 1);

}

void LoadImageTask::run()
{
    _canceled=false;

    if (!_image)
    {
//        _manager->loadFinished(0);
        return;
    }

    QString path = _image->getFileToOpen();

    ImageLayer * layer = 0;
    layer = new ImageLayer();
    if (!layer->setup(path))
    {
        ClearData();
        _manager->loadFinished(0);
        return;
    }

    // setup image info :
    setupImageInfo(layer, _image);

    Cancel();

    _manager->loadProgressValueChanged(15);

    // create an overview:
    _reporter->startValue=15;
    _reporter->endValue=74;
    createOverviews(layer->getDataset(), _reporter);

    Cancel();

    _manager->loadProgressValueChanged(75);

    // compute histogram
    _reporter->startValue=75;
    _reporter->endValue=99;
    if (!computeNormalizedHistogram(layer, 1000, true, _reporter))
    {
        ClearData();
        _manager->loadFinished(0);
        return;
    }

    Cancel();

    _manager->loadProgressValueChanged(100);
    _manager->_map.insert(_image, layer);
    _manager->loadFinished(_image);
}

//******************************************************************************

//void LoadImageTask::setImagePath(const QString *path)
//{
//    _canceled=true;
//    _path = path;
//}

//******************************************************************************

void LoadImageTask::setImage(Image *image)
{
    _canceled=true;
    _image = image;
}

//******************************************************************************

}

