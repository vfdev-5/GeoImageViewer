#ifndef LASSOPLUGIN_H
#define LASSOPLUGIN_H

// Qt
#include <QObject>
#include <QEvent>

// Project
#include "Lasso.h"
#include "Tools/AbstractTool.h"
#include "Tools/RectangleTool.h"

namespace Plugins
{

//******************************************************************************

class GIV_PLUGIN_EXPORT LassoPlugin : public Tools::RectangleTool
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "AbstractTool")

public:

    LassoPlugin(QObject * parent = 0) :
        Tools::RectangleTool(parent)
    {
        _name=tr("Lasso");
        _description=tr("Lasso tool to select a region");
        _icon = QIcon(":/icons/selection");
        _cursor = QCursor(Qt::CrossCursor);

        _pen = QPen(QColor(Qt::gray), 0, Qt::DashLine);
        _brush = QBrush(QColor(Qt::transparent));

    }

    virtual bool dispatch(QEvent * e, QGraphicsScene * scene);

protected:



};

//******************************************************************************

}

#endif // LASSOPLUGIN_H
