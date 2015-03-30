#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H



// Project
#include "Core/Global.h"
#include "Core/LibExport.h"
#include "AbstractTool.h"

class QAbstractGraphicsShapeItem;

namespace Tools
{

//******************************************************************************

class GIV_DLL_EXPORT BrushTool : public AbstractTool
{
    Q_OBJECT

    Q_PROPERTY_WITH_ACCESSORS(QColor, color, getColor, setColor)
    Q_PROPERTY_WITH_ACCESSORS(int, size, getSize, setSize)

    Q_CLASSINFO("color","label:Color")
    Q_CLASSINFO("size","label:Size;minValue:1;maxValue:250")

public:
    explicit BrushTool(QObject *parent = 0);

    virtual bool dispatch(QEvent * e, QGraphicsScene * scene);
    virtual bool dispatch(QEvent * e, QWidget * viewport);


protected:

    QAbstractGraphicsShapeItem * _cursorShape;

};

//******************************************************************************

}

#endif // BRUSHTOOL_H
