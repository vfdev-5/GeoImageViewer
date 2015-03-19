#ifndef FILTERSMANAGER_H
#define FILTERSMANAGER_H

// Qt
#include <QString>
#include <QHash>
#include <QObject>

// Project
#include "Core/LibExport.h"
#include "AbstractFilter.h"

namespace Core
{
class ImageDataProvider;
}


namespace Filters
{

class FilterTask;

//******************************************************************************


class GIV_DLL_EXPORT FiltersManager : public QObject
{
    Q_OBJECT
    friend class FilterTask;
public:

    void loadPlugins(const QString & path);

    static FiltersManager * get()
    {
        if (!_instance)
            _instance = new FiltersManager();
        return _instance;
    }

    static void destroy()
    {
        if (_instance) {
            delete _instance;
        }
    }

    AbstractFilter* getFilter(const QString & name)
    { return _filters.value(name, 0); }

    QList<AbstractFilter*> getFilters()
    { return _list; }

    void insertFilter(AbstractFilter * filter);


    void applyFilterInBackground(const AbstractFilter * filter, const Core::ImageDataProvider *provider);
    Core::ImageDataProvider * applyFilter(AbstractFilter * filter);
    void cancel();
    bool isWorking()
    { return _isWorking; }

signals:
    void filteringFinished(Core::ImageDataProvider * provider);
    void filterProgressValueChanged(int);


private:
    FiltersManager();
    ~FiltersManager();
    static FiltersManager * _instance;

    bool loadPlugin(QObject *plugin);

    QHash<QString, AbstractFilter*> _filters;
    QList<AbstractFilter*> _list;

    FilterTask * _task;
    void taskFinished(Core::ImageDataProvider * provider);
    bool _isAsyncTask;
    bool _isWorking;

};

//******************************************************************************

}

#endif // FILTERSMANAGER_H
