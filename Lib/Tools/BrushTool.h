#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H



// Project
#include "Core/Global.h"
#include "Core/LibExport.h"
#include "AbstractTool.h"

class QAbstractGraphicsShapeItem;
class QGraphicsView;

namespace Tools
{

//******************************************************************************

class GIV_DLL_EXPORT BrushTool : public ImageCreationTool
{
    Q_OBJECT

    Q_PROPERTY_WITH_ACCESSORS(QColor, color, getColor, setColor)

//    Q_PROPERTY(bool erase READ erase WRITE setErase)
//    PROPERTY_GETACCESSOR(bool, erase, erase)
//    Q_CLASSINFO("erase","label:Erase")


    PROPERTY_ACCESSORS(double, size, getSize, setSize)
    Q_CLASSINFO("color","label:Color")


public:
    explicit BrushTool(QGraphicsScene* scene, QGraphicsView * view, QObject *parent = 0);

    virtual bool dispatch(QEvent * e, QGraphicsScene * scene);
    virtual bool dispatch(QEvent * e, QWidget * viewport);

    virtual void setErase(bool erase);

signals:
    void sizeChanged(double);

protected:

    void createCursor();
    void destroyCursor();
    void drawCircle(const QRectF &r);
    void drawAtPoint(const QPointF & pt);

    QAbstractGraphicsShapeItem * _cursorShape;
    QGraphicsScene * _scene;
    QGraphicsView * _view;
    QColor _ucolor;
//    int _mode;

};

//******************************************************************************

}

#endif // BRUSHTOOL_H
