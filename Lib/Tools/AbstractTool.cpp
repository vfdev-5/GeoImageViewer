// Qt
#include <QGraphicsScene>
#include <QPainter>

// Project
#include "AbstractTool.h"


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
    CreationTool(parent)
{
    _toolType = Type;
}

//******************************************************************************

ImageCreationTool::ImageCreationTool(QObject * parent) :
    CreationTool(parent),
    _drawingsItem(0),
    _mode(QPainter::RasterOp_SourceOrDestination),
    _erase(false),
    _isMerging(true)
{
    _toolType = Type;
}

//******************************************************************************

void ImageCreationTool::setErase(bool erase)
{
    _erase = erase;
    _mode = _erase ? QPainter::CompositionMode_SourceOut :
                     _isMerging ? QPainter::RasterOp_SourceOrDestination:
                                  QPainter::CompositionMode_SourceOver;
}

//******************************************************************************

void ImageCreationTool::setIsMerging(bool value)
{
    _isMerging = value;
    _mode = _isMerging ? QPainter::RasterOp_SourceOrDestination : QPainter::CompositionMode_SourceOver;
}

//******************************************************************************

}
