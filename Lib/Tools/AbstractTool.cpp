
// Project
#include "AbstractTool.h"

#include <QGraphicsEllipseItem>

namespace Tools
{

//******************************************************************************
/*!
  \class AbstractTool
  \brief Abstract class represent application tools, e.g navigation, move, create a shape, etc

  \class ItemCreationTool
  \brief Inherits from AbstractTool, abstract class represents application tools that create a shape and notify
  the application that a new BaseLayer is created

  */

//******************************************************************************

AbstractTool::AbstractTool(QObject *parent) :
    QObject(parent),
    _toolType(Type)
{

}

//******************************************************************************

ItemCreationTool::ItemCreationTool(QObject *parent) :
    AbstractTool(parent)
{
    _toolType = Type;
}

//******************************************************************************

}
