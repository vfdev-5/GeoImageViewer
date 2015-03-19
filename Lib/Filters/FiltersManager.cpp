
// Qt
#include <QDir>
#include <QPluginLoader>
#include <QRunnable>
#include <QThreadPool>

// Project
#include "FiltersManager.h"
#include "BlurFilter.h"
#include "Core/Global.h"
#include "Core/ImageDataProvider.h"
#include "Core/FloatingDataProvider.h"
#include "Core/LayerUtils.h"

namespace Filters
{

class FilterTask : public QRunnable
{
public:
    FilterTask():
        _canceled(false),
        _srcProvider(0),
        _dstProvider(0),
        _filter(0)
    {
        setAutoDelete(true);
    }

    virtual void run();

    void setSource(const Core::ImageDataProvider * p)
    {
        _canceled = true;
        _srcProvider = p;
    }

    void setOutput(Core::FloatingDataProvider * o)
    { _dstProvider = o; }

    Core::ImageDataProvider * getOutput()
    { return _dstProvider; }

    void setFilter(const AbstractFilter * filter)
    {
        _canceled = true;
        _filter = filter;
    }

protected:
    bool _canceled;
    const AbstractFilter * _filter;
    const Core::ImageDataProvider * _srcProvider;
    Core::FloatingDataProvider * _dstProvider;
};


FiltersManager * FiltersManager::_instance = 0;

//******************************************************************************

FiltersManager::FiltersManager() :
    _task(0),
    _isWorking(false),
    _isAsyncTask(true)
{
    // Insert default filters :
    insertFilter(new BlurFilter());
}

//******************************************************************************

FiltersManager::~FiltersManager()
{
}

//******************************************************************************

void FiltersManager::insertFilter(AbstractFilter *filter)
{
    filter->setParent(this);
    _filters.insert(filter->objectName(), filter);
    _list << filter;
}


//******************************************************************************

void FiltersManager::loadPlugins(const QString &path)
{
    QDir d(path);
    QStringList filters;
#ifdef _DEBUG
    filters << "*Plugin.d.dll" << "*Plugin.d.so";
#else
    filters << "*Plugin.dll" << "*Plugin.so";
#endif

    foreach (QString fileName, d.entryList(filters, QDir::Files))
    {
        QPluginLoader loader(d.absoluteFilePath(fileName));
        QObject * plugin = loader.instance();
        if (!plugin || !loadPlugin(plugin))
        {
            SD_TRACE(loader.errorString());
            SD_TRACE("Failed to load plugin : " + fileName);
        }
    }
}

//******************************************************************************

bool FiltersManager::loadPlugin(QObject *plugin)
{
    AbstractFilter * filter = qobject_cast<AbstractFilter*>(plugin);
    if (!filter)
        return false;

    _filters.insert(filter->objectName(), filter);
    _list << filter;

    SD_TRACE("Filter loaded : " + filter->getName());
    return true;
}

//******************************************************************************

Core::ImageDataProvider * FiltersManager::applyFilter(AbstractFilter *filter)
{
    return 0;
}

//******************************************************************************

void FiltersManager::applyFilterInBackground(const AbstractFilter *filter, const Core::ImageDataProvider *provider)
{
    // task is automatically deleted on finished
    _task = new FilterTask();

    _task->setFilter(filter);
    _task->setSource(provider);
    _task->setOutput(new Core::FloatingDataProvider());
    _isAsyncTask = true;

    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    pool->waitForDone();
    // Only one thread is possible due to GDAL reader (e.g. TIFF)
    _isWorking=true;
    pool->start(_task);
}

//******************************************************************************

void FiltersManager::cancel()
{
    _task->setFilter(0);
    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    pool->waitForDone();
    _isWorking=false;
}

//******************************************************************************

void FiltersManager::taskFinished(Core::ImageDataProvider * provider)
{
    _isWorking=false;
    if (_isAsyncTask)
        emit filteringFinished(provider);
}

//******************************************************************************
//******************************************************************************

#define ClearData() \
    if (_dstProvider) { \
        delete _dstProvider; \
        _dstProvider = 0;   \
    }


#define Cancel() \
    if (_canceled) { \
        ClearData(); \
        return; \
    }

void FilterTask::run()
{
    _canceled=false;
    if (!_filter || !_srcProvider)
    {
        ClearData();
        FiltersManager::get()->taskFinished(0);
        return;
    }

    if (_srcProvider->getWidth() > 2048 && _srcProvider->getHeight() > 2048)
    {
        SD_TRACE("FilterTask::run : src is too large");
        ClearData();
        FiltersManager::get()->taskFinished(0);
        return;
    }

    Cancel();

    FiltersManager::get()->filterProgressValueChanged(5);

    cv::Mat dstMat;
    {
        cv::Mat srcMat = _srcProvider->getImageData();
        dstMat = _filter->apply(srcMat);
    }

    if (dstMat.empty())
    {
        ClearData();
        FiltersManager::get()->taskFinished(0);
        return;
    }

    FiltersManager::get()->filterProgressValueChanged(60);

    _dstProvider->create(_srcProvider->getImageName() + " + " + _filter->getName(), dstMat);

    FiltersManager::get()->filterProgressValueChanged(100);
    FiltersManager::get()->taskFinished(_dstProvider);
}

//******************************************************************************

}
