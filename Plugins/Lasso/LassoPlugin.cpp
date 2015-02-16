
// Qt
#include <QGraphicsSceneMouseEvent>

// Project
#include "LassoPlugin.h"
#include "Core/Global.h"

namespace Plugins
{

//******************************************************************************

bool LassoPlugin::dispatch(QEvent * e, QGraphicsScene * scene)
{
    if (e->type() == QEvent::GraphicsSceneMousePress)
    {
        return mousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(e), scene);
    }
    else if (e->type() == QEvent::GraphicsSceneMouseMove)
    {
        return mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(e), scene);
    }
    else if (e->type() == QEvent::GraphicsSceneMouseRelease)
    {
        return mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(e), scene);
    }

    return false;
}

//******************************************************************************

}
