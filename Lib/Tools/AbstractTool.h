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
#include "Core/ImageDataProvider.h"
#include "Core/DrawingsItem.h"

class QGraphicsScene;
class QAbstractGraphicsShapeItem;
class QGraphicsPixmapItem;
class QGraphicsView;
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
    PROPERTY_GETACCESSOR(QIcon, icon, getIcon)
    PROPERTY_GETACCESSOR(QCursor, cursor, getCursor)
    PROPERTY_GETACCESSOR(QList<QAction*>, actions, getActions)

public:
    AbstractTool(QObject * parent = 0);
    virtual ~AbstractTool() {}
    virtual bool dispatch(QEvent * e, QGraphicsScene * scene) = 0;
    virtual bool dispatch(QEvent * e, QWidget * graphicsViewViewport)
    { Q_UNUSED(e); Q_UNUSED(graphicsViewViewport); return false; }

public slots:
    virtual void clear() {}

public:
    enum {
        Type = 0,
    };

    bool hasActions() const
    { return !_actions.isEmpty(); }
};

//******************************************************************************

class GIV_DLL_EXPORT CreationTool : public AbstractTool
{
    Q_OBJECT
public:
    CreationTool(QObject * parent = 0) :
        AbstractTool(parent),
        _pressed(false)
    {}

protected:
    bool _pressed;
    QPointF _anchor;
};

//******************************************************************************

class GIV_DLL_EXPORT ItemCreationTool : public CreationTool
{
    Q_OBJECT
public:

    ItemCreationTool(QObject * parent = 0);
    enum {
        Type = 1,
    };


signals:
    void itemCreated(QGraphicsItem * item);

};

//******************************************************************************

class GIV_DLL_EXPORT ImageCreationTool : public CreationTool
{
    Q_OBJECT

    PTR_PROPERTY_ACCESSORS(Core::DrawingsItem, drawingsItem, getDrawingsItem, setDrawingsItem)

    Q_PROPERTY(bool erase READ erase WRITE setErase)
    PROPERTY_GETACCESSOR(bool, erase, erase)
    Q_CLASSINFO("erase","label:Erase")

    Q_PROPERTY(bool isMerging READ isMerging WRITE setIsMerging)
    PROPERTY_GETACCESSOR(bool, isMerging, isMerging)
    Q_CLASSINFO("isMerging", "label:Merge")


public:

    ImageCreationTool(QObject * parent = 0);
    enum {
        Type = 2,
    };

    virtual void setErase(bool erase);
    virtual void setIsMerging(bool value);

protected:
    int _mode;
};

//******************************************************************************

}

#endif // AbstractTool_H
