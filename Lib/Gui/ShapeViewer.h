#ifndef SHAPEVIEWER_H
#define SHAPEVIEWER_H


// Qt
#include <QHash>
#include <QStandardItemModel>

// Project
#include "Core/LibExport.h"
#include "BaseViewer.h"


class QGraphicsItem;

namespace Tools {
class ToolsManager;
class AbstractTool;
}

namespace Core {
class BaseLayer;
}

namespace Gui
{

class AbstractToolsView;
class LayersView;

//******************************************************************************

class GIV_DLL_EXPORT ShapeViewer : public BaseViewer
{
    Q_OBJECT

public:

    ShapeViewer(const QString & initialText = QString(), QWidget * parent = 0);
    virtual ~ShapeViewer();

    virtual void clear();

    void setToolsView(AbstractToolsView * toolsView);
    void setLayersView(LayersView * layersView);

protected slots:
    virtual void onToolChanged(QString);
    virtual void onItemCreated(QGraphicsItem*);
    virtual void onBaseLayerStateChanged();
    virtual void onBaseLayerDestroyed(QObject *);
    virtual void onBaseLayerSelected(Core::BaseLayer*);
    virtual void onSaveBaseLayer(Core::BaseLayer*);

protected:
    virtual bool eventFilter(QObject *, QEvent *);

    void addLayer(Core::BaseLayer*, QGraphicsItem *item);
    bool removeLayer(Core::BaseLayer* layer);

    Tools::ToolsManager * _toolsManager;
    Tools::AbstractTool * _currentTool;
    AbstractToolsView * _toolsView;

    bool _enableTools;

    QHash<Core::BaseLayer*, QGraphicsItem*> _layerItemMap;
//    QStandardItemModel _layers;

    LayersView * _layersView;


};

//******************************************************************************

}

#endif // SHAPEVIEWER_H
