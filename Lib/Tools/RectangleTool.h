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

class GIV_DLL_EXPORT RectangleTool : public AbstractTool
{

    Q_OBJECT

    Q_PROPERTY_WITH_ACCESSORS(QString, testString, getTestString, setTestStringa)

    Q_PROPERTY_WITH_ACCESSORS(QPen, pen, getPen, setPen)
    Q_PROPERTY_WITH_ACCESSORS(QBrush, brush, getBrush, setBrush)
    PROPERTY_ACCESSORS(bool, singleItem, isSingleItem, setIsSingleItem)

public:
    RectangleTool(QObject * parent);
    virtual ~RectangleTool() {}
    virtual bool dispatch(QEvent * e, QGraphicsScene * scene);
    virtual void clear(QGraphicsScene * scene);

protected:
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *e, QGraphicsScene * scene);
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent * e, QGraphicsScene * scene);
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent * e, QGraphicsScene * scene);

    bool _pressed;
    QGraphicsRectItem * _rect;
    QPointF _anchor;

};

//******************************************************************************

}

#endif // RECTANGLETOOL_H
