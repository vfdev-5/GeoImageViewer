
// Qt
#include <QPixmap>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QWheelEvent>
#include <QPoint>

// Project
#include "BrushTool.h"

namespace Tools
{

//******************************************************************************

BrushTool::BrushTool(QObject *parent) :
    AbstractTool(parent),
    _color(Qt::red),
    _size(100),
    _cursorShape(0)
{
    setObjectName("brush");
    _name=tr("Brush");
    _description=tr("Tool to draw on a layer");
    _icon = QIcon(":/icons/brush");

    QGraphicsEllipseItem * item = new QGraphicsEllipseItem(QRectF(-0.5,-0.5, 1.0, 1.0));
    item->setPen(QPen(_color, 0));
    item->setBrush(_color);
    item->setScale(_size);
    item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    _cursorShape = item;

}

//******************************************************************************

bool BrushTool::dispatch(QEvent *e, QGraphicsScene *scene)
{

    if (e->type() == QEvent::GraphicsSceneMouseMove)
    {
        if (!_cursorShape->scene())
        {
            scene->addItem(_cursorShape);
        }

        QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
        _cursorShape->setPos(event->scenePos());

        return true;
    }
    return false;
}

//******************************************************************************

bool BrushTool::dispatch(QEvent *e, QWidget *viewport)
{

    if (e->type() == QEvent::Enter)
    {
//        SD_TRACE("QEvent::Enter");
        if (!_cursorShape->isVisible())
        {
            _cursorShape->setVisible(true);
            viewport->setCursor(Qt::BlankCursor);
            _cursorShape->setPen(QPen(_color, 0));
            _cursorShape->setBrush(_color);
            _cursorShape->setScale(_size);
        }
        return true;
    }
    else if (e->type() == QEvent::Leave)
    {
//        SD_TRACE("QEvent::Leave");
        if (_cursorShape->isVisible())
        {
            _cursorShape->setVisible(false);
            viewport->setCursor(QCursor());
        }
        return true;
    }
    else if (e->type() == QEvent::Wheel)
    {
        QWheelEvent * event = static_cast<QWheelEvent*>(e);
        // mouse has X units that covers 360 degrees. Zoom when 15 degrees is provided
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QPoint numDegrees = event->angleDelta() / 8;
#else
        QPoint numDegrees(0,event->delta() / 8);
#endif
        if (!numDegrees.isNull())
        {
            if (numDegrees.y() >= 15)
            { // make smaller
                _size *= 0.6667;
                _size = _size < 1.0 ? 1.0 : _size;
            }
            else if (numDegrees.y() <= -15)
            { // make larger
                _size *= 1.5;
                _size = _size > 250.0 ? 250.0 : _size;
            }
            // !!! SHOULD NOTIFY PROPERTY EDITOR THAT OBJECT PROPERTY VALUE IS CHANGED !!!

            _cursorShape->setScale(_size);
            event->accept();
            return true;
        }
    }
    return false;
}

//******************************************************************************

}
