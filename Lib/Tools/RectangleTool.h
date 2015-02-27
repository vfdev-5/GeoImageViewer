#ifndef RECTANGLETOOL_H
#define RECTANGLETOOL_H


// Qt
#include <QPen>
#include <QBrush>
#include <QPointF>

// Project
#include "Core/Global.h"
#include "Core/LibExport.h"
#include "AbstractTool.h"

class QGraphicsRectItem;
class QGraphicsSceneMouseEvent;

namespace Tools
{

//******************************************************************************

class GIV_DLL_EXPORT RectangleTool : public ItemCreationTool
{

    Q_OBJECT

    PROPERTY_ACCESSORS(QPen, pen, getPen, setPen)
    PROPERTY_ACCESSORS(QBrush, brush, getBrush, setBrush)

public:
    RectangleTool(QObject * parent);
    virtual ~RectangleTool() {}
    virtual bool dispatch(QEvent * e, QGraphicsScene * scene);

public slots:
    virtual void clear();

protected:
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *e, QGraphicsScene * scene);
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent * e, QGraphicsScene * scene);
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent * e, QGraphicsScene * scene);

    bool _pressed;
    QGraphicsRectItem * _rect;
    QPointF _anchor;

};

//******************************************************************************

class GIV_DLL_EXPORT RectangleShapeTool : public RectangleTool
{
    Q_OBJECT
    Q_PROPERTY(QPen pen READ getPen WRITE setPen)
    Q_PROPERTY(QBrush brush READ getBrush WRITE setBrush)

    Q_CLASSINFO("pen","label:Outline style")
    Q_CLASSINFO("brush","label:Fill style")

public :
    RectangleShapeTool(QObject * parent) :
        RectangleTool(parent)
    {}

};


//******************************************************************************

}

#endif // RECTANGLETOOL_H
