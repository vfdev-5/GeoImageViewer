
// Qt
#include <QDir>
#include <QPluginLoader>

// Project
#include "Core/Global.h"
#include "ToolsManager.h"
#include "RectangleTool.h"

namespace Tools
{

ToolsManager * ToolsManager::_instance = 0;

//******************************************************************************

ToolsManager::ToolsManager()
{
    Q_INIT_RESOURCE(resources);

    // Insert default tools :
    NavigationTool * tool = new NavigationTool(this);
    _tools.insert(tool->getName(), tool);
    _list << tool;


    Tools::RectangleTool * rTool = new Tools::RectangleTool(this);
    _tools.insert(rTool->getName(), rTool);
    _list << rTool;


}

//******************************************************************************

ToolsManager::~ToolsManager()
{
//    foreach (QString key, _tools.keys())
//    {
//        AbstractTool * tool = _tools.value(key);
//        if (tool)
//            delete tool;
//    }
}

//******************************************************************************

void ToolsManager::loadPlugins(const QString &path)
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

bool ToolsManager::loadPlugin(QObject *plugin)
{
    AbstractTool * tool = qobject_cast<AbstractTool*>(plugin);
    if (!tool)
        return false;

    _tools.insert(tool->getName(), tool);
    _list << tool;

    SD_TRACE("Tool loaded : " + tool->getName());
    return true;
}

//******************************************************************************

//QList<ToolInfo> ToolsManager::getTools() const
//{
//    QList<ToolInfo> out;
//    foreach (AbstractTool * atool, _list)
//    {
//        out << *(ToolInfo*)atool;
//    }
//    return out;
//}

//******************************************************************************

}
