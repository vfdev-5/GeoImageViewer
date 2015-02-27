#ifndef MOVETOOL_H
#define MOVETOOL_H

// Project
#include "AbstractTool.h"
#include "Core/LibExport.h"
#include "Core/Global.h"

class QGraphicsItem;

namespace Tools
{

//******************************************************************************

class GIV_DLL_EXPORT MoveTool : public AbstractTool
{
    Q_OBJECT
public:
    explicit MoveTool(QObject *parent = 0);
    virtual ~MoveTool() {}
    virtual bool dispatch(QEvent * e, QGraphicsScene * scene);


protected:
    bool _pressed;
    QGraphicsItem * _movingItem;
};

//******************************************************************************

}

#endif // MOVETOOL_H
