
// Qt
#include <QGraphicsItem>
#include <QLabel>
#include <QGraphicsSceneMouseEvent>
#include <qmath.h>

// Project
#include "ShapeViewer.h"
#include "AbstractToolsView.h"
#include "LayersView.h"
#include "Core/Global.h"
#include "Core/GeoShapeLayer.h"
#include "Tools/ToolsManager.h"
#include "Tools/AbstractTool.h"


namespace Gui
{

double computeZValue(int index)
{
    return 10.0 + 2.0 * index;
}

//******************************************************************************
/*!
* \class ShapeViewer
*
* \brief This class provides a reimplementation of BaseViewer with integrations of tools.
*/
//******************************************************************************

ShapeViewer::ShapeViewer(const QString &initialText, QWidget *parent) :
    BaseViewer(initialText, parent),
    _currentTool(0),
    _toolsView(0),
    _layersView(0),
    _enableTools(true)
{
    // init ToolsManager
    _toolsManager = Tools::ToolsManager::get();
}

//******************************************************************************

ShapeViewer::~ShapeViewer()
{
    _currentTool = 0;
    Tools::ToolsManager::destroy();
}

//******************************************************************************

void ShapeViewer::clear()
{
    SD_TRACE("ShapeViewer::clear");
    foreach (Core::BaseLayer * layer, _layers)
    {
        layer->setParent(0);
        disconnect(layer, 0, 0, 0);
        removeItem(layer);
        delete layer;
    }
    _layers.clear();
    _layerItemMap.clear();

    if (_layersView)
        _layersView->setLayers(_layers);

    BaseViewer::clear();
}

//******************************************************************************

void ShapeViewer::onToolChanged(const QString & toolName)
{
    SD_TRACE("onToolChanged : " + toolName);
    Tools::AbstractTool * tool = _toolsManager->getTool(toolName);
    // change to the new one
    if (!tool)
    {
        SD_TRACE("onToolChanged : failed to find the tool by name : " + toolName);
        return;
    }

    changeTool(tool);
}

//******************************************************************************

void ShapeViewer::changeTool(Tools::AbstractTool *newTool)
{
    // disable previous tool :
    if (_currentTool)
    {
        // disconnect 'item => layer' link
        if (_currentTool->getType() == Tools::ItemCreationTool::Type)
        {
            disconnect(_currentTool, SIGNAL(itemCreated(QGraphicsItem*)), this, SLOT(onItemCreated(QGraphicsItem*)));
        }

        // remove actions:
        if (_currentTool->hasActions())
        {
            foreach (QAction * a, _currentTool->getActions())
            {
                _menu.removeAction(a);
            }
        }
    }

    // setup new tool :
    _currentTool = newTool;
    if (_currentTool->getType() == Tools::ItemCreationTool::Type)
    {
        connect(_currentTool, SIGNAL(itemCreated(QGraphicsItem*)), this, SLOT(onItemCreated(QGraphicsItem*)));
    }

    setCurrentCursor(_currentTool->getCursor());

    // add actions to menu:
    if (_currentTool->hasActions())
    {
//        SD_TRACE("Add tool actions : nb= " + QString::number(_currentTool->getActions().size())
//                 + " | menu.actionsNb= " + QString::number(_menu.actions().size()));
        _menu.addSeparator();
        _menu.addActions(_currentTool->getActions());
    }
}

//******************************************************************************

void ShapeViewer::onItemCreated(QGraphicsItem * item)
{
    SD_TRACE("onItemCreated");
    // create layer:
    Core::BaseLayer * layer = new Core::BaseLayer(this);
    layer->setType(_currentTool->getName());

    addLayer(layer, item);
}

//******************************************************************************

void ShapeViewer::onBaseLayerStateChanged()
{
    Core::BaseLayer * layer = qobject_cast<Core::BaseLayer*>(sender());
    if (!layer)
        return;

    QGraphicsItem * item = _layerItemMap.value(layer, 0);
    if (!item)
    {
        SD_TRACE("onBaseLayerStateChanged : graphics item associated to base layer is not found");
        return;
    }

    bool isVisible = item->isVisible();
    double opacity = item->opacity();
    double zValue = item->zValue();

    if (isVisible != layer->isVisible())
    {
        item->setVisible(layer->isVisible());
    }

    if (opacity != layer->getOpacity())
    {
        item->setOpacity(layer->getOpacity());
    }

    if (zValue != layer->getZValue())
    {
        double z = computeZValue(layer->getZValue());
        item->setZValue(
                    computeZValue(
                        layer->getZValue())
                    );
    }

}

//******************************************************************************

void ShapeViewer::onBaseLayerDestroyed(QObject * layerObj)
{
    Core::BaseLayer * layer = static_cast<Core::BaseLayer*>(layerObj);
    if (!layer)
        return;

    if (!removeItem(layer))
    {
        SD_TRACE("ShapeViewer::onBaseLayerDestroyed : failed to remove layer");
        return;
    }

    // remove layer from the map :
    _layerItemMap.remove(layer);
    _layers.removeAll(layer);

    // if the last layer destroyed -> clear
    if (_layers.isEmpty())
    {
        clear();
    }

}

//******************************************************************************

void ShapeViewer::onBaseLayerSelected(Core::BaseLayer * layer)
{
    Q_UNUSED(layer);
    SD_TRACE("ShapeViewer::onBaseLayerSelected");
}

//******************************************************************************

void ShapeViewer::onCreateBaseLayer()
{
    // Nothing to do
}

//******************************************************************************

void ShapeViewer::onSaveBaseLayer(Core::BaseLayer * layer)
{
    Q_UNUSED(layer);
    SD_TRACE("ShapeViewer::onSaveBaseLayer");
}

//******************************************************************************
/*
 * Event filter :
 * 1) Use tools
 */
bool ShapeViewer::eventFilter(QObject * o, QEvent * e)
{
    if (o == &_scene)
    {
        if (_enableTools &&
                _currentTool &&
                _currentTool->dispatch(e, &_scene))
        {
            e->accept();
            return true;
        }
        // display current point info :
        if (e->type() == QEvent::GraphicsSceneMouseMove)
        {
            if (_pointInfo->isVisible())
            {
                QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
//                QPointF pt = computePointOnItem(event->scenePos());
//                if (pt.x() >= 0.0 || pt.y() >= 0.0)
                {
                    QPointF pt = event->scenePos();
                    QString info = QString("Pixel coordinates : %1, %2")
                            .arg(pt.x())
                            .arg(pt.y());
                    bool isComplex = false;
                    QVector<double> vals = getPixelValues(QPoint(qFloor(pt.x()), qFloor(pt.y())) , &isComplex);
                    if (!vals.isEmpty())
                    {
                        info += " | Pixel value : ";
                        if (isComplex)
                            info += "Complex ";
                        info += "( ";
                        for (int i=0; i<vals.size();i++)
                        {
                            double v = vals[i];
                            info += QString("%1 ").arg(v);
                        }
                        info += ")";
                    }
                    _pointInfo->setText(info);

                }
            }
        }
    }
    else if (o == _view.viewport())
    {
        if (_enableTools &&
                _currentTool &&
                _currentTool->dispatch(e, _view.viewport()))
        {
            e->accept();
            return true;
        }
    }

    return BaseViewer::eventFilter(o, e);
}

//******************************************************************************

Core::BaseLayer *ShapeViewer::getCurrentLayer() const
{
    if (_layersView)
        return _layersView->getCurrentLayer();
    if (!_layers.isEmpty())
        return _layers.last();
    return 0;
}

//******************************************************************************

void ShapeViewer::addLayer(Core::BaseLayer * layer, QGraphicsItem * item)
{

    // create layer:
    connect(layer, SIGNAL(layerStateChanged()), this, SLOT(onBaseLayerStateChanged()));
    connect(layer, SIGNAL(destroyed(QObject*)), this, SLOT(onBaseLayerDestroyed(QObject*)));


    _layers.append(layer);
    _layerItemMap.insert(layer, item);

    if (_layersView)
    {
        _layersView->addLayer(layer);
    }
    else
    {
        int count = _layers.size()-1;
        item->setZValue(computeZValue(count));
        layer->setZValue(count);
    }
}

//******************************************************************************

void ShapeViewer::setToolsView(AbstractToolsView *toolsView)
{

    if (_toolsView)
    {
        disconnect(_toolsView, SIGNAL(toolChanged(QString)), this, SLOT(onToolChanged(QString)));
    }

    _toolsView = toolsView;
    if (!_toolsView)
        return;

    connect(_toolsView, SIGNAL(toolChanged(QString)), this, SLOT(onToolChanged(QString)));

    QList<Tools::AbstractTool*> list = _toolsManager->getTools();
    foreach (Tools::AbstractTool* t, list)
    {
        _toolsView->addTool(t);
    }
    _toolsView->setCurrentTool(list.first()->objectName());

}

//******************************************************************************

void ShapeViewer::setLayersView(LayersView *view)
{
    if (_layersView)
    {
        disconnect(_layersView, SIGNAL(layerSelected(Core::BaseLayer*)),
                this, SLOT(onBaseLayerSelected(Core::BaseLayer*)));
        disconnect(_layersView, SIGNAL(saveLayer(Core::BaseLayer*)),
                this, SLOT(onSaveBaseLayer(Core::BaseLayer*)));
        disconnect(_layersView, SIGNAL(createNewLayer()),
                this, SLOT(onCreateBaseLayer()));

    }

    _layersView = view;
    if (!_layersView)
        return;

    // connect :
    connect(_layersView, SIGNAL(layerSelected(Core::BaseLayer*)),
            this, SLOT(onBaseLayerSelected(Core::BaseLayer*)));
    connect(_layersView, SIGNAL(saveLayer(Core::BaseLayer*)),
            this, SLOT(onSaveBaseLayer(Core::BaseLayer*)));
    connect(_layersView, SIGNAL(createNewLayer()),
            this, SLOT(onCreateBaseLayer()));

}

//******************************************************************************

bool ShapeViewer::removeItem(Core::BaseLayer *layer)
{
    if (!layer)
    {
        SD_TRACE("ShapeViewer::removeItem : base layer is null");
        return false;
    }

    // remove graphics item:
    QGraphicsItem * item = _layerItemMap.value(layer, 0);
    if (!item)
    {
        SD_TRACE("ShapeViewer::removeItem : no graphics item associated with base layer");
        return false;
    }

    _scene.removeItem(item);
    delete item;

    return true;
}

//******************************************************************************

QPointF ShapeViewer::computePointOnItem(const QPointF &scenePos)
{
//    QGraphicsItem * item = _scene.itemAt(scenePos, QTransform());
//    if (item)
//    {
//    }
    return QPointF(-1.0, -1.0);
}

//******************************************************************************

}
