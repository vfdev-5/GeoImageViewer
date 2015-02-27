
// Qt
#include <QDir>
#include <QPluginLoader>

// Project
#include "Core/Global.h"
#include "ToolsManager.h"
#include "RectangleTool.h"
#include "SelectionTool.h"
#include "MoveTool.h"


inline void initResources() { Q_INIT_RESOURCE(resources); }

namespace Tools
{

ToolsManager * ToolsManager::_instance = 0;

//******************************************************************************

ToolsManager::ToolsManager()
{

    initResources();

    // Insert default tools :
    insertTool(new NavigationTool(this));
    insertTool(new MoveTool(this));
    insertTool(new RectangleShapeTool(this));
//    insertTool(new SelectionTool(this));
}

//******************************************************************************

ToolsManager::~ToolsManager()
{
}

//******************************************************************************

void ToolsManager::insertTool(AbstractTool *tool)
{
    tool->setParent(this);
    _tools.insert(tool->objectName(), tool);
    _list << tool;
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

    _tools.insert(tool->objectName(), tool);
    _list << tool;

    SD_TRACE("Tool loaded : " + tool->getName());
    return true;
}

//******************************************************************************

}
