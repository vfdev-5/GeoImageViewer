
// Qt
#include <QThreadPool>
#include <QFileInfo>

// GDAL
#include <gdal_priv.h>

// Project
#include "ImageOpener.h"
#include "ImageDataProvider.h"
#include "Gui/SubdatasetDialog.h"

namespace Core
{

//******************************************************************************
/*!
  \class ImageOpener
  \brief is a helper to open an image from different kind of sources: local, network, ...
  Image is opened in a separate thread. The result of the operation is provided with the signal
  imageOpened(ImageDataProvider*).

 */
//******************************************************************************

ImageOpener::ImageOpener(QObject *parent) :
    QObject(parent),
    _task(0),
    _isAsyncTask(false),
    _isWorking(false)
{
    int count = GetGDALDriverManager()->GetDriverCount();
    if (count == 0)
        GDALAllRegister();
}

//******************************************************************************

ImageOpener::~ImageOpener()
{
    int count = GetGDALDriverManager()->GetDriverCount();
    if (count > 0)
        GDALDestroyDriverManager();
}

//******************************************************************************

ImageDataProvider * ImageOpener::openImage(const QUrl & url)
{
    if (url.isLocalFile())
    {
        QString filepath = url.toLocalFile();
        QString fileToOpen = handleLocalFile(url);

        if (!_task)
            _task = new OpenImageFileTask(this);

        _task->setImage(fileToOpen);
        // provider will be parented to the recepient of the signal imageOpened
        GDALDataProvider * provider = new GDALDataProvider();
        provider->setImageName(QFileInfo(filepath).baseName());
        _task->setDataProvider(provider);
        _isAsyncTask = false;
        _task->run();

        return _task->getDataProvider();

    }
    else
    {
        // DO SOMETHING ELSE
        SD_TRACE("ImageOpener : url is not local path -> return");
        return 0;
    }




}

//******************************************************************************

QString ImageOpener::handleLocalFile(const QUrl &url)
{
    QString filepath = url.toLocalFile();
    // pre-loading phase :
    if (!QFileInfo(filepath).exists())
    {
        SD_ERR(tr("File %1 is not found").arg(filepath));
        return QString();
    }

    QString fileToOpen=filepath;
    // check if image has subsets
    QStringList subsetNames, subsetDesriptions;
    if (Core::isSubsetFile(fileToOpen, subsetNames, subsetDesriptions))
    { // Image has subsets
        if (subsetNames.size() > 1)
        { // Image has more than one subset -> ask user to choose
            Gui::SubdatasetDialog dialog(subsetDesriptions);
            if (!dialog.exec())
            {
                return QString();
            }
            else
            {
                int index = dialog.getSelectionIndex();
                if (index < 0 || index >= subsetNames.size())
                    return QString();
                fileToOpen = subsetNames[index];
            }
        }
        else
        {
            fileToOpen = subsetNames.first();
        }
    }
    return fileToOpen;
}


//******************************************************************************

bool ImageOpener::openImageInBackground(const QUrl &url)
{
    if (url.isLocalFile())
    {
        QString filepath = url.toLocalFile();
        QString fileToOpen = handleLocalFile(url);
        if (!_task)
            _task = new OpenImageFileTask(this);
        _task->setImage(fileToOpen);
        // provider will be parented to the recepient of the signal imageOpened
        GDALDataProvider * provider = new GDALDataProvider();
        provider->setImageName(QFileInfo(filepath).baseName());
        _task->setDataProvider(provider);
        _isAsyncTask = true;
    }
    else
    {
        // DO SOMETHING ELSE
        SD_TRACE("ImageOpener : url is not local path -> return");
        return false;
    }

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

void ImageOpener::cancel()
{
    _task->setImage(QString());
    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    pool->waitForDone();
    _isWorking=false;
}

//******************************************************************************

void ImageOpener::taskFinished(ImageDataProvider * provider)
{
    _isWorking=false;
    if (_isAsyncTask)
        emit imageOpened(provider);
}

//******************************************************************************
//******************************************************************************

OpenImageFileTask::OpenImageFileTask(ImageOpener *parent) :
    OpenImageTask(parent)
{
}

//******************************************************************************

#define ClearData() \
    if (provider) { \
        delete provider; \
        provider = 0;   \
    }


#define Cancel() \
    if (_canceled) { \
        ClearData() \
        return; \
    }

void OpenImageFileTask::run()
{
    _canceled=false;

    GDALDataProvider * provider = qobject_cast<GDALDataProvider*>(_dataProvider);

    if (_path.isEmpty() || provider == 0)
    {
        _imageOpener->taskFinished(0);
        return;
    }

    if (!provider->setup(_path))
    {
        ClearData();
        _imageOpener->taskFinished(0);
        return;
    }

    Cancel();

    _imageOpener->openProgressValueChanged(15);

    // create an overview:
    _reporter->startValue=15;
    _reporter->endValue=74;
    createOverviews(provider->getDataset(), _reporter);

    Cancel();

    _imageOpener->openProgressValueChanged(75);

    // compute histogram
    _reporter->startValue=75;
    _reporter->endValue=99;
    QVector<double> minValues;
    QVector<double> maxValues;
    QVector< QVector<double> > bandHistograms;
    cv::Mat data = provider->getImageData(QRect(), 1024);
    if (data.empty())
    {
        ClearData();
        _imageOpener->taskFinished(0);
        return;
    }

    // compute NoDataMask :
    cv::Mat mask = ImageDataProvider::computeMask(data);

    int inputDepth = provider->getInputDepthInBytes();
    int histSize = inputDepth > 1 ? 1000 : 256;
    if (!computeNormalizedHistogram(data, mask,
                                minValues, maxValues,
                                bandHistograms,
                                histSize, _reporter) )
    {
        ClearData();
        _imageOpener->taskFinished(0);
        return;
    }

    // if input image depth is 8 bits -> min/max values should be [0,255] for all bands
    if (inputDepth == 1)
    {
        int nbBands = data.channels();
        minValues.clear();
        minValues.fill(0, nbBands);
        maxValues.clear();
        maxValues.fill(255, nbBands);
    }


    provider->setMinValues(minValues);
    provider->setMaxValues(maxValues);
    provider->setBandHistograms(bandHistograms);

    Cancel();

    _imageOpener->openProgressValueChanged(100);
    _imageOpener->taskFinished(provider);

}

//******************************************************************************

}
