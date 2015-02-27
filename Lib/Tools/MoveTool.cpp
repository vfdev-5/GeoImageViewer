
// Qt
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

// Project
#include "MoveTool.h"

namespace Tools
{

//******************************************************************************

MoveTool::MoveTool(QObject *parent) :
    AbstractTool(parent),
    _pressed(false),
    _movingItem(0)
{
    setObjectName("move");
    _name=tr("Move");
    _description=tr("Tool to move objects");
    _icon = QIcon(":/icons/move");
    _cursor = QCursor(Qt::SizeAllCursor);
}

//******************************************************************************

bool MoveTool::dispatch(QEvent * e, QGraphicsScene * scene)
{
    if (e->type() == QEvent::GraphicsSceneMousePress)
    {
        QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
        if (event->button() == Qt::LeftButton && !_pressed)
        {
//            SD_TRACE("MoveTool : mouse press" );
            _pressed = true;
            QGraphicsItem * item = scene->itemAt(event->scenePos(), QTransform());

            if (item != 0)
            {
//                SD_TRACE("MoveTool : item is under cursor" );
                QVariant b = item->data(QGraphicsItem::ItemIsMovable);
                if (b.isValid() && b.toBool())
                {
//                    SD_TRACE("MoveTool : item is movable" );
                    item->setFlag(QGraphicsItem::ItemIsMovable);
                    _movingItem = item;
                }
            }
        }
    }
    else if (e->type() == QEvent::GraphicsSceneMouseRelease)
    {
        QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
        if (event->button() == Qt::LeftButton && _pressed)
        {
//            SD_TRACE("MoveTool : mouse release" );
            _pressed=false;
            if (_movingItem)
            {
                _movingItem->setFlag(QGraphicsItem::ItemIsMovable, false);
                _movingItem = 0;
            }
        }
    }
    return false;
}

//******************************************************************************

}
