
// Qt
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>

// Project
#include "RectangleTool.h"

namespace Tools
{

//******************************************************************************


RectangleTool::RectangleTool(QObject *parent) :
    ItemCreationTool(parent),
    _pressed(false),
    _pen(QColor(Qt::black), 0, Qt::SolidLine),
    _brush(QColor(Qt::white)),
    _rect(0)
{
    setObjectName("rectangle");
    _name = tr("Rectangle");
    _description = tr("Draws a rectangle shape");
    _icon = QIcon(":/icons/rectangle");
}

//******************************************************************************

bool RectangleTool::dispatch(QEvent * e, QGraphicsScene * scene)
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

bool RectangleTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene)
{

    if (event->button() == Qt::LeftButton && !_pressed)
    {
//        SD_TRACE("RectangleTool : mouse press" );
//        if (_singleItem)
//        {
//            clear(scene);
//        }

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

bool RectangleTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *)
{
    if (_pressed)
    {
//        SD_TRACE("RectangleTool : mouse move" );
        QPointF p1 = event->scenePos();

        QRectF r;

        if (p1.x() > _anchor.x())
        {
            r.setLeft(_anchor.x());
            r.setRight(p1.x());
        }
        else
        {
            r.setLeft(p1.x());
            r.setRight(_anchor.x());
        }

        if (p1.y() > _anchor.y())
        {
            r.setTop(_anchor.y());
            r.setBottom(p1.y());
        }
        else
        {
            r.setTop(p1.y());
            r.setBottom(_anchor.y());
        }

        _rect->setRect(r);

        return true;
    }

    return false;
}

//******************************************************************************

bool RectangleTool::mouseReleaseEvent(QGraphicsSceneMouseEvent * event, QGraphicsScene *)
{
    if (event->button() == Qt::LeftButton && _pressed)
    {
//        SD_TRACE("RectangleTool : mouse release" );
        _pressed=false;
        _anchor = QPointF();

        emit itemCreated(_rect);

        _rect = 0;
//        if (!_singleItem) {
//            _rect = 0;
//        }

        return true;
    }
    return false;
}

//******************************************************************************

void RectangleTool::clear()
{
    if (_rect && !_pressed)
    {
        _rect->scene()->removeItem(_rect);
        delete _rect;
        _rect=0;
    }
}

//******************************************************************************

}
