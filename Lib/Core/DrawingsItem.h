#ifndef DRAWINGSITEM_H
#define DRAWINGSITEM_H

// Qt
#include <QGraphicsItem>
#include <QImage>

// Project
#include "Global.h"


namespace Core
{

//******************************************************************************

class DrawingsItem : public QGraphicsItem
{

    PROPERTY_GETACCESSOR(QColor, background, getBackground)

public:
    DrawingsItem(int width, int height, QColor bg = QColor(127,127,127,50), QGraphicsItem * parent = 0);

    enum { Type = UserType + 3 };
    int type() const { return Type; }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *o, QWidget * w);

    QImage & getImage()
    { return _image; }

protected:
    QImage _image;
};

//******************************************************************************

}

#endif // DRAWINGSITEM_H
