#ifndef ABSTRACTTOOL_H
#define ABSTRACTTOOL_H

// Qt
#include <QObject>
#include <QIcon>
#include <QEvent>
#include <QCursor>

// Project
#include "Core/LibExport.h"
#include "Core/Global.h"

class QGraphicsScene;

namespace Tools
{

//******************************************************************************

class GIV_DLL_EXPORT AbstractTool : public QObject
{
    Q_OBJECT
    Q_PROPERTY_WITH_GETACCESSOR(QString, name, getName)
    Q_PROPERTY_WITH_GETACCESSOR(QString, description, getDescription)
    Q_PROPERTY_WITH_GETACCESSOR(QIcon, icon, getIcon)
    Q_PROPERTY_WITH_GETACCESSOR(QCursor, cursor, getCursor)

public:
    AbstractTool(QObject * parent);
    virtual ~AbstractTool() {}
    virtual bool dispatch(QEvent * e, QGraphicsScene * scene) = 0;
    virtual void clear(QGraphicsScene * scene) = 0;

protected:


};

//******************************************************************************

}

//QT_BEGIN_NAMESPACE
//Q_DECLARE_INTERFACE(Core::AbstractTool, "AbstractTool")
//QT_END_NAMESPACE

#endif // AbstractTool_H
