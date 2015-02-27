#ifndef SELECTIONTOOL_H
#define SELECTIONTOOL_H

// Qt
#include <QObject>
#include <QEvent>
#include <QAction>

// Project
#include "Core/Global.h"
#include "Core/LibExport.h"
#include "RectangleTool.h"

namespace Tools
{

//******************************************************************************

class SelectionTool : public Tools::RectangleTool
{
    Q_OBJECT
public:
    SelectionTool(QObject * parent = 0);
    virtual bool dispatch(QEvent * e, QGraphicsScene * scene);

public slots:
    virtual void clear();

protected:
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *e, QGraphicsScene * scene);
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent * e, QGraphicsScene * scene);

protected slots:
    void onCopyToNewLayerTriggered();

signals:
    void copyToNewLayer(const QRectF & selection);

private:
    QAction * _clearSelection;
    QAction * _toNewLayer;

};

//******************************************************************************

}

#endif // SELECTIONTOOL_H
