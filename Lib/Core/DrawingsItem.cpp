
// Qt
#include <QPainter>

// Project
#include "Global.h"
#include "DrawingsItem.h"


namespace Core
{

//******************************************************************************

/*!
  \class DrawingsItem
  \brief inherits from QGraphicsItem and serves as a canvas (contains a QImage) for drawing shapes using QPainter.
  For example :
    DrawingsItem d(200, 300, QColor(Qt::white));
    QPainter p(&d.getImage());
    p.drawLine(...)
    etc
    d.update()

  DrawingsItem::boundingRect() return a rectangle of the QImage size

 */

//******************************************************************************

DrawingsItem::DrawingsItem(int width, int height, QColor bg, QGraphicsItem * parent) :
    QGraphicsItem(parent),
    _background(bg)
{
    _image = QImage(width, height, QImage::Format_ARGB32);
    _image.fill(_background);
}

//******************************************************************************

QRectF DrawingsItem::boundingRect() const
{
    return QRectF(QPointF(0.0, 0.0), _image.size());
}

//******************************************************************************

void DrawingsItem::paint(QPainter * p, const QStyleOptionGraphicsItem *o, QWidget * w)
{
    Q_UNUSED(o);
    Q_UNUSED(w);
    //    SD_TRACE(" > DrawingsItem::paint");
    QRectF r = boundingRect();
    p->drawImage(r, _image, r);
}

//******************************************************************************

}
