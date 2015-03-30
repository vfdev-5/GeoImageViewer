
// Qt
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

// Project
#include "SelectionTool.h"

namespace Tools
{

//******************************************************************************

SelectionTool::SelectionTool(QObject * parent) :
    Tools::RectangleTool(parent)
{
    setObjectName("selection");
    _name=tr("Selection");
    _description=tr("Selection tool to select a region");
    _icon = QIcon(":/icons/selection");
    _cursor = QCursor(Qt::CrossCursor);

    _pen = QPen(QColor(Qt::gray), 0, Qt::DashLine);
    _brush = QBrush(QColor(Qt::transparent));

    // Actions:
    _clearSelection = new QAction(tr("Clear selection"), this);
    _clearSelection->setEnabled(false);
    connect(_clearSelection, SIGNAL(triggered()), this, SLOT(clear()));
    _actions.append(_clearSelection);

    _toNewLayer = new QAction(tr("Copy to new layer"), this);
    _toNewLayer->setEnabled(false);
    connect(_toNewLayer, SIGNAL(triggered()), this, SLOT(onCopyToNewLayerTriggered()));
    _actions.append(_toNewLayer);

}

//******************************************************************************

bool SelectionTool::dispatch(QEvent * e, QGraphicsScene * scene)
{
    if (e->type() == QEvent::GraphicsSceneMousePress)
    {
        return mousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(e), scene);
    }
    else if (e->type() == QEvent::GraphicsSceneMouseMove)
    {
        return mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(e), scene);
    }
    else if (e->type() == QEvent::GraphicsSceneMouseRelease)
    {
        return mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(e), scene);
    }

    return false;
}

//******************************************************************************

void SelectionTool::clear()
{
    if (_rect && !_pressed)
    {
        _clearSelection->setEnabled(false);
        _toNewLayer->setEnabled(false);
    }
    RectangleTool::clear();
}

//******************************************************************************

bool SelectionTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene)
{

    if (event->button() == Qt::LeftButton && !_pressed)
    {
        if (_rect)
        {
            clear();
        }

        _pressed = true;
        _anchor = event->scenePos();
        QRectF r(_anchor, QSizeF(0.5,0.5));
        _rect = scene->addRect(r, _pen, _brush);
        _rect->setZValue(1000);
        _rect->setData(QGraphicsItem::ItemIsMovable, true);

        return true;
    }
    return false;
}

//******************************************************************************

bool SelectionTool::mouseReleaseEvent(QGraphicsSceneMouseEvent * event, QGraphicsScene *)
{
    if (event->button() == Qt::LeftButton && _pressed)
    {
        _pressed=false;
        _anchor = QPointF();

        _clearSelection->setEnabled(true);
        _toNewLayer->setEnabled(true);

        return true;
    }
    return false;
}

//******************************************************************************

void SelectionTool::onCopyToNewLayerTriggered()
{
    if (_rect)
    {
        emit copyToNewLayer(_rect->rect());
//        clear();
    }
}

//******************************************************************************

}
