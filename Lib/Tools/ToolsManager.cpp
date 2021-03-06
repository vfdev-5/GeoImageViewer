
// Qt
#include <QDir>
#include <QPluginLoader>

// Project
#include "Core/Global.h"
#include "Core/PluginLoader.h"
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
    foreach (Core::Plugin pair, Core::PluginLoader::loadAll(path))
    {
        QString fileName = pair.first;
        QObject * plugin = pair.second;
        if (!plugin || !loadPlugin(plugin))
        {
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
