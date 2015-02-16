
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
    AbstractTool(parent),
    _pressed(false),
    _pen(QColor(Qt::gray), 0, Qt::DashLine),
    _brush(QColor(Qt::transparent)),
    _singleItem(true),
    _rect(0),
    _testString("Test value")
{
    _name = "Rectangle";
    _description = "Draws a rectangle shape";
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
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(e), scene);
        return true;
    }

    return false;

}

//******************************************************************************

bool RectangleTool::mousePressEvent(QGraphicsSceneMouseEvent *event, QGraphicsScene *scene)
{

    if (event->button() == Qt::LeftButton)
    {
//        SD_TRACE("RectangleTool : mouse press" );
        _pressed = true;
        if (_singleItem && _rect)
        {
            scene->removeItem(_rect);
            delete _rect;
            _rect=0;
        }
        _anchor = event->scenePos();
        QRectF r(_anchor, QSizeF(0.5,0.5));
        _rect = scene->addRect(r, _pen, _brush);
        _rect->setZValue(1000);

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
    if (event->button() == Qt::LeftButton)
    {
//        SD_TRACE("RectangleTool : mouse release" );
        _pressed=false;
        _anchor = QPointF();

//        if (!_singleItem) {
//            _rect = 0;
//        }

        return true;
    }
    return false;
}

//******************************************************************************

void RectangleTool::clear(QGraphicsScene * scene)
{
    if (_rect)
    {
        scene->removeItem(_rect);
        delete _rect;
        _rect=0;
    }
}

//******************************************************************************

}
