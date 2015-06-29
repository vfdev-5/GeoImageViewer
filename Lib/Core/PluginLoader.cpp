
// Qt
#include <QDir>
#include <QPluginLoader>


// Project
#include "PluginLoader.h"
#include "Global.h"


namespace Core
{
#ifdef _DEBUG
    const QStringList PluginLoader::PluginFilters = QStringList() << "*Plugin.d.dll" << "*Plugin.d.so" << "*Plugin.d.dylib";
#else
    const QStringList PluginLoader::PluginFilters = QStringList() << "*Plugin.dll" << "*Plugin.so" << "*Plugin.dylib";
#endif

QList<Plugin> PluginLoader::loadAll(const QString &path)
{
    QList<QPair<QString, QObject *> > out;

    QDir d(path);
    foreach (QString fileName, d.entryList(PluginFilters, QDir::Files))
    {
        SD_TRACE("Plugin filename : " + fileName);
        QPluginLoader loader(d.absoluteFilePath(fileName));
        QObject * plugin = loader.instance();
        out << QPair<QString,QObject*>(fileName, plugin);
    }
    return out;
}

QObject * PluginLoader::load(const QString & pluginPath, QString & errorMessage)
{
    QPluginLoader loader(pluginPath);
    QObject * plugin = loader.instance();
    errorMessage = loader.errorString();
    return plugin;
}


}
