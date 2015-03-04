
// Qt
#include <QDir>
#include <QPluginLoader>

// Project
#include "Core/Global.h"
#include "FiltersManager.h"
#include "BlurFilter.h"

namespace Filters
{

FiltersManager * FiltersManager::_instance = 0;

//******************************************************************************

FiltersManager::FiltersManager()
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
    foreach (QString fileName, d.entryList(QStringList() << "*.dll", QDir::Files))
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

}
