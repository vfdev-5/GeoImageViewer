
// Qt
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QWheelEvent>
#include <QPoint>
#include <qmath.h>

// Project
#include "BrushTool.h"
#include "Core/LayerUtils.h"

namespace Tools
{

//******************************************************************************

/*!
  \class BrushTool
  \brief Class inherits from ImageCreationTool class and represent a tool to draw circles on Core::DrawingsItem.
  User can specify the color and the size.
*/

//******************************************************************************

BrushTool::BrushTool(QGraphicsScene* scene, QGraphicsView * view, QObject *parent) :
    ImageCreationTool(parent),
    _color(Qt::red),
    _ucolor(_color),
    _size(100),
    _cursorShape(0),
    _scene(scene),
    _view(view)
{
    setObjectName("brush");
    _name=tr("Brush");
    _description=tr("Tool to draw on a layer");
    _icon = QIcon(":/icons/brush");
}

//******************************************************************************

bool BrushTool::dispatch(QEvent *e, QGraphicsScene *scene)
{

    if (e->type() == QEvent::GraphicsSceneMousePress)
    {
        QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
        if (event->button() == Qt::LeftButton && !_pressed && _drawingsItem)
        {
            _pressed = true;
            _anchor = event->scenePos();
            drawAtPoint(_anchor);
            return true;
        }
    }
    else if (e->type() == QEvent::GraphicsSceneMouseMove)
    {
        QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
        if (_cursorShape)
            _cursorShape->setPos(event->scenePos());

        if (_pressed && (event->buttons() & Qt::LeftButton) && _drawingsItem)
        {
            drawAtPoint(event->scenePos());
        }
        // Allow to process other options on mouse move. Otherwise should return true
        return false;
    }
    else if (e->type() == QEvent::GraphicsSceneMouseRelease)
    {
        QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
        if (event->button() == Qt::LeftButton && _pressed)
        {
            _pressed=false;
            _anchor = QPointF();

            return true;
        }
        return false;
    }
    return false;
}

//******************************************************************************

bool BrushTool::dispatch(QEvent *e, QWidget *viewport)
{

    if (e->type() == QEvent::Enter)
    {
        // This handle the situation when GraphicsScene was cleared between Enter and Leave events
        if (!_cursorShape)
        {
            createCursor();
            viewport->setCursor(_cursor);
        }
        return true;
    }
    else if (e->type() == QEvent::Leave)
    {
        // This handle the situation when GraphicsScene was cleared between Enter and Leave events
        if (_cursorShape)
        {
            destroyCursor();
            viewport->setCursor(QCursor());
        }

        return true;
    }
    else if (e->type() == QEvent::Wheel)
    {
        QWheelEvent * event = static_cast<QWheelEvent*>(e);
        if (event->modifiers() & Qt::ControlModifier)
        {
            if (_cursorShape)
            {
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
                    emit sizeChanged(_size);

                    _cursorShape->setScale(_size);
                    event->accept();
                    return true;
                }
            }
        }
    }
    return false;
}

//******************************************************************************

void BrushTool::destroyCursor()
{
    QGraphicsScene * scene = _cursorShape->scene();
    if (scene)
    {
        scene->removeItem(_cursorShape);
        delete _cursorShape;
        _cursorShape = 0;
    }
}

//******************************************************************************

void BrushTool::createCursor()
{
//    if (!_cursorShape)
//    {
    if (!_erase && _ucolor != _color)
    {
        _ucolor = _color;
    }
    QGraphicsEllipseItem * item = new QGraphicsEllipseItem(QRectF(-0.5,-0.5, 1.0, 1.0));
    item->setPen(QPen(_ucolor, 0));
    item->setBrush(_ucolor);
    item->setScale(_size);
    item->setZValue(1000);
    item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    _cursorShape = item;
    _scene->addItem(_cursorShape);
    //    }
}

//******************************************************************************

void BrushTool::drawAtPoint(const QPointF &pos)
{
    // check if the rectangle to paint is in the drawingItem zone:
    QRectF r = _drawingsItem->boundingRect();
    r.moveTo(_drawingsItem->pos());
    double size = _size / qMin(_view->matrix().m11(), _view->matrix().m22());
    QRectF r2 = QRectF(pos.x()-size*0.5, pos.y()-size*0.5, size, size);
    if (r.intersects(QRectF(pos.x()-_size*0.5, pos.y()-_size*0.5, _size, _size)))
    {
        r2.translate(-_drawingsItem->scenePos());
        drawCircle(r2);
        r2=r2.adjusted(-size*0.25, -size*0.25, size*0.25, size*0.25);
        _drawingsItem->update(r2);
    }
}

//******************************************************************************

void BrushTool::drawCircle(const QRectF & r)
{
    QPainter p(&_drawingsItem->getImage());
    p.setCompositionMode((QPainter::CompositionMode)_mode);
    p.setPen(QPen(_ucolor, 0.0));
    p.setBrush(_ucolor);
    p.drawEllipse(r);
}

//******************************************************************************

void BrushTool::setErase(bool erase)
{
    ImageCreationTool::setErase(erase);
    if (_erase && _drawingsItem)
    {
        _ucolor = _drawingsItem->getBackground();
    }
    else
    {
        _ucolor = _color;
    }
}

//******************************************************************************
/*!
 * \brief BrushTool::clear Clears the tool's temporary objects
 * This method should be called before QGraphicsScene::clear().
 */
void BrushTool::clear()
{
    if (_cursorShape)
    {
        destroyCursor();
    }
}

//******************************************************************************

}
