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

struct GIV_DLL_EXPORT PluginLoader
{
    static const QStringList PluginFilters;
    static QList< Plugin > loadAll(const QString &path);
    static QObject * load(const QString & pluginPath, QString & errorMessage);
};

}

#endif // PLUGINLOADER_H
