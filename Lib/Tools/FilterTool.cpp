
// Qt
#include <QPainter>
#include <QAction>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QWheelEvent>
#include <QPoint>
#include <qmath.h>

// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "FilterTool.h"
#include "Core/LayerUtils.h"

namespace Tools
{

//******************************************************************************

/*!
  \class FilterTool
  \brief Abstract class inherits from ImageCreationTool class and represent a base structure
  to filter image part under the mouse in real-time mode.
  Abstract method processData() should be subclassed to process input data under the mouse.
  Virtual method onFinalize() should be subclassed to finish the processing and send the result.
  The result of the filtering can be a QGraphicsItem or Core::DrawingsItem which is not owned by the class.


*/

//******************************************************************************


FilterTool::FilterTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent):
    ImageCreationTool(parent),
    _scene(scene),
    _view(view),
    _dataProvider(0),
    _cursorShape(0),
    _cursorShapeScale(-1),
    _cursorShapeZValue(1000),
    _size(100),
    _isValid(false),
    _opacity(0.5)
{
    _toolType = Type;

    _finalize = new QAction(tr("Finalize"), this);
    _finalize->setEnabled(false);
    connect(_finalize, SIGNAL(triggered()), this, SLOT(onFinalize()));
    _actions.append(_finalize);

    _hideShowResult = new QAction(tr("Hide result"), this);
    _hideShowResult->setEnabled(false);
    connect(_hideShowResult, SIGNAL(triggered()), this, SLOT(onHideShowResult()));
    _actions.append(_hideShowResult);

    QAction * separator = new QAction(this);
    separator->setSeparator(true);
    _actions.append(separator);

    _clear = new QAction(tr("Clear"), this);
    _clear->setEnabled(false);
    connect(_clear, SIGNAL(triggered()), this, SLOT(clear()));
    _actions.append(_clear);

    _isValid = _scene && _view;

}

//******************************************************************************

bool FilterTool::dispatch(QEvent *e, QGraphicsScene *scene)
{
    if (!_isValid)
        return false;

    if (e->type() == QEvent::GraphicsSceneMousePress)
    {

        QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
        if (event->button() == Qt::LeftButton &&
                !_pressed &&
                !(event->modifiers() & Qt::ControlModifier))
        {
            // check if _drawingsItem exists
            if (!_drawingsItem)
            {
                // create drawings item for the current visible extent
                QRect wr = _view->viewport()->rect();
                QRectF r = _view->mapToScene(wr).boundingRect();
                if (r.width() < 2000 && r.height() < 2000)
                {
                    _drawingsItem = new Core::DrawingsItem(r.width(), r.height(), QColor(Qt::transparent));
                    _drawingsItem->setPos(r.x(), r.y());
                    _drawingsItem->setZValue(_cursorShapeZValue-10);
                    _drawingsItem->setOpacity(_opacity);
                    // visible canvas
                    QGraphicsRectItem * canvas = new QGraphicsRectItem(QRectF(0.0,0.0,r.width(), r.height()), _drawingsItem);
                    canvas->setPen(QPen(Qt::white, 0));
                    canvas->setBrush(QBrush(QColor(127,127,127,50)));
                    canvas->setFlag(QGraphicsItem::ItemStacksBehindParent);
                    scene->addItem(_drawingsItem);
                    _finalize->setEnabled(true);
                    _clear->setEnabled(true);
                    _hideShowResult->setEnabled(true);
                }
                else
                {
                    SD_WARN("Visible zone is too large. Zoom to define smaller zone");
                }
            }

            if (_drawingsItem && _cursorShape)
            {
                _pressed = true;
                _anchor = event->scenePos();

                drawAtPoint(event->scenePos());
                return true;
            }
        }
    }
    else if (e->type() == QEvent::GraphicsSceneMouseMove)
    {
        QGraphicsSceneMouseEvent * event = static_cast<QGraphicsSceneMouseEvent*>(e);
        if (_cursorShape)
        {
            double size = _size / qMin(_view->matrix().m11(), _view->matrix().m22());
            QPointF pos = event->scenePos();
            _cursorShape->setPos(pos - QPointF(0.5*size, 0.5*size));

            if (_dataProvider && !_erase)
            {
                size *= _cursorShape->scale();
                pos -= QPointF(0.5*size, 0.5*size);

                //                SD_TRACE(QString("rect : %1, %2, %3, %4")
                //                         .arg(pos.x())
                //                         .arg(pos.y())
                //                         .arg(size)
                //                         .arg(size));
                QRect r(pos.x(), pos.y(), size, size);
                cv::Mat data = _dataProvider->getImageData(r);
                if (!data.empty())
                {
                    cv::Mat out = processData(data);
                    //                    SD_TRACE(QString("out : %1, %2, %3, %4")
                    //                             .arg(out.rows)
                    //                             .arg(out.cols)
                    //                             .arg(out.channels())
                    //                             .arg(out.elemSize1()));

                    QPixmap p = QPixmap::fromImage(QImage(out.data,
                                                          out.cols,
                                                          out.rows,
                                                          QImage::Format_ARGB32).copy());
                    p = p.scaled(QSize(_size,_size));
                    _cursorShape->setPixmap(p);
                }
            }

        }

        if (_pressed && (event->buttons() & Qt::LeftButton)
                && _drawingsItem && _cursorShape)
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

bool FilterTool::dispatch(QEvent *e, QWidget *viewport)
{
    if (!_isValid)
        return false;

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
        if (event->modifiers() & Qt::ControlModifier )
        {
            if (_cursorShape)
            {
                // mouse has X units that covers 360 degrees. Zoom when 15 degrees is provided
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
                QPoint numDegrees = event->angleDelta() / 8;
#else
                QPoint numDegrees(0,event->delta() / 8);
#endif
                double scale = _cursorShape->scale();
                if (!numDegrees.isNull())
                {
                    if (numDegrees.y() >= 15)
                    { // make smaller
                        scale *= 0.6667;
                        scale = scale < 0.2 ? 0.2 : scale;
                    }
                    else if (numDegrees.y() <= -15)
                    { // make larger
                        scale *= 1.5;
                        scale = scale > 5.0 ? 5.0 : scale;
                    }
                    //                emit sizeChanged(_size);

                    _cursorShape->setScale(scale);

                    event->accept();
                    return true;
                }
            }
        }
    }
    return false;
}



//******************************************************************************

void FilterTool::createCursor()
{
//    if (!_cursorShape)
//    {
    QPixmap p = QPixmap(_size,_size);
    p.fill(_drawingsItem ? _drawingsItem->getBackground() : Qt::transparent);

    QGraphicsPixmapItem * item = new QGraphicsPixmapItem(p);
    item->setTransformOriginPoint(0.5*_size,0.5*_size);
    item->setZValue(_cursorShapeZValue);
    item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    item->setOpacity(_opacity);

    QGraphicsRectItem * br = new QGraphicsRectItem(item->boundingRect(), item);
    br->setZValue(+1);
    br->setPen(QPen(Qt::green, 0));
    br->setBrush(Qt::transparent);
    br->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);


    _cursorShape = item;
    if (_cursorShapeScale>0)
        _cursorShape->setScale(_cursorShapeScale);
    _scene->addItem(_cursorShape);
//    }
}

//******************************************************************************

void FilterTool::destroyCursor()
{
    _cursorShapeScale = _cursorShape->scale();
    QGraphicsScene * scene = _cursorShape->scene();
    if (scene)
    {
        scene->removeItem(_cursorShape);
        delete _cursorShape;
        _cursorShape = 0;
    }
}

//******************************************************************************

void FilterTool::onHideShowResult()
{
    if (_drawingsItem->isVisible())
    {
        _drawingsItem->setVisible(false);
        _hideShowResult->setText(tr("Show result"));
    }
    else
    {
        _drawingsItem->setVisible(true);
        _hideShowResult->setText(tr("Hide result"));
    }
}

//******************************************************************************

void FilterTool::onFinalize()
{
    _drawingsItem = 0;
    _finalize->setEnabled(false);
    _clear->setEnabled(false);
    _hideShowResult->setEnabled(false);
}

//******************************************************************************

void FilterTool::clear()
{
    if (_cursorShape)
    {
        destroyCursor();
        _cursorShapeScale = -1.0;
    }
    if (_drawingsItem)
    {
        _scene->removeItem(_drawingsItem);
        delete _drawingsItem;
    }
    FilterTool::onFinalize();
}

//******************************************************************************

void FilterTool::setErase(bool erase)
{
    ImageCreationTool::setErase(erase);
}

//******************************************************************************

void FilterTool::drawAtPoint(const QPointF &pos)
{
    // check if the rectangle to paint is in the drawingItem zone:
    QRectF r = _drawingsItem->boundingRect();
    r.moveTo(_drawingsItem->pos());
    double size = _size * _cursorShape->scale() / qMin(_view->matrix().m11(), _view->matrix().m22());
    QRectF r2 = QRectF(pos.x()-size*0.5, pos.y()-size*0.5, size, size);

    if (r.intersects(QRectF(pos.x()-_size*0.5, pos.y()-_size*0.5, _size, _size)))
    {
        r2.translate(-_drawingsItem->scenePos());

        QPainter p(&_drawingsItem->getImage());
        p.setCompositionMode((QPainter::CompositionMode)_mode);
        QPixmap px = _cursorShape->pixmap();
        p.drawPixmap(r2, px, QRectF(0.0,0.0,px.width(),px.height()));

        r2=r2.adjusted(-size*0.25, -size*0.25, size*0.25, size*0.25);
        _drawingsItem->update(r2);
    }
}

//******************************************************************************

}
