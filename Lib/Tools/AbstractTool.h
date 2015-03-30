#ifndef ABSTRACTTOOL_H
#define ABSTRACTTOOL_H

// Qt
#include <QObject>
#include <QIcon>
#include <QEvent>
#include <QCursor>
#include <QList>

// Project
#include "Core/LibExport.h"
#include "Core/Global.h"

class QGraphicsScene;
class QGraphicsItem;
class QAction;

namespace Tools
{

//******************************************************************************

class GIV_DLL_EXPORT AbstractTool : public QObject
{
    Q_OBJECT
    PROPERTY_GETACCESSOR(int, toolType, getType)
    Q_PROPERTY_WITH_GETACCESSOR(QString, name, getName)
    Q_PROPERTY_WITH_GETACCESSOR(QString, description, getDescription)
    Q_PROPERTY_WITH_GETACCESSOR(QIcon, icon, getIcon)
    Q_PROPERTY_WITH_GETACCESSOR(QCursor, cursor, getCursor)

    PROPERTY_GETACCESSOR(QList<QAction*>, actions, getActions)

public:
    AbstractTool(QObject * parent = 0);
    virtual ~AbstractTool() {}
    virtual bool dispatch(QEvent * e, QGraphicsScene * scene) = 0;
    virtual bool dispatch(QEvent * e, QWidget * graphicsViewViewport)
    { Q_UNUSED(e); Q_UNUSED(graphicsViewViewport); return false; }

    enum {
        Type = 0,
    };

    bool hasActions() const
    { return !_actions.isEmpty(); }
};

//******************************************************************************

class GIV_DLL_EXPORT ItemCreationTool : public AbstractTool
{
    Q_OBJECT
public:

    ItemCreationTool(QObject * parent = 0);
    enum {
        Type = 1,
    };

public slots:
    virtual void clear() = 0;


signals:
    void itemCreated(QGraphicsItem * item);

};

//******************************************************************************

class GIV_DLL_EXPORT ImageCreationTool : public AbstractTool
{
    Q_OBJECT
public:

    ImageCreationTool(QObject * parent = 0) :
        AbstractTool(parent)
    {
        _toolType = Type;
    }
    enum {
        Type = 2,
    };

//public slots:
//    virtual void clear() = 0;

//signals:
//    void itemCreated(QGraphicsItem * item);

};

//******************************************************************************

}

#endif // AbstractTool_H
