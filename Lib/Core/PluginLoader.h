#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H


// Qt
#include <QPair>
#include <QList>
#include <QObject>
#include <QStringList>

// Project
#include "LibExport.h"

namespace Core
{
typedef QPair<QString,QObject*> Plugin;

struct PluginLoader
{
    static const QStringList PluginFilters;
    static QList< Plugin > GIV_DLL_EXPORT loadAll(const QString &path);
    static QObject * GIV_DLL_EXPORT load(const QString & pluginPath, QString & errorMessage);
};

}

#endif // PLUGINLOADER_H
