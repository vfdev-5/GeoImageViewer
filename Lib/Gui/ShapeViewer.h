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
class GeoShapeLayer;
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
    virtual void onToolChanged(const QString &);
    virtual void onItemCreated(QGraphicsItem*);
    virtual void onBaseLayerDestroyed(QObject *);
    virtual void onBaseLayerSelected(Core::BaseLayer*);
    virtual void onSaveBaseLayer(Core::BaseLayer*);
    virtual void onCreateBaseLayer();

protected:
    virtual bool eventFilter(QObject *, QEvent *);

    void changeTool(Tools::AbstractTool * newTool);
    void addLayer(Core::BaseLayer*);

    virtual QPointF computePointOnItem(const QPointF &scenePos);

    Core::BaseLayer *getCurrentLayer() const;
    Core::GeoShapeLayer * createGeoShapeLayer(const QString &name, QGraphicsItem *item, const Core::GeoShapeLayer *background=0);

    virtual QPolygonF computeGeoExtentFromLayer(const QPolygonF & inputShape, const Core::GeoShapeLayer * backgroundLayer);

    Tools::ToolsManager * _toolsManager;
    Tools::AbstractTool * _currentTool;
    AbstractToolsView * _toolsView;

    bool _enableTools;

    QList<Core::BaseLayer*> _layers;

//    Core::BaseLayer* _rootLayer;

    LayersView * _layersView;


};

//******************************************************************************

}

#endif // SHAPEVIEWER_H
