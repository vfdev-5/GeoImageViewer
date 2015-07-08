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
  \brief Abstract class represent application tools, e.g navigation, move, create a shape, image etc

  \class ItemCreationTool
  \brief Inherits from CreationTool, abstract class represents application tools that create a shape (QGraphicsRectItem, QGraphicsLineItem, etc)
   and notify the application that a new BaseLayer is created. Created QGraphicsItem is own by QGraphicsScene.

  \class ImageCreationTool from CreationTool,
  \brief Inherits from CreationTool, abstract class represents a tool to create a Core::DrawingsItem. Created Core::DrawingsItem is not owned by this class

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
    _cursor = QCursor(Qt::BlankCursor);
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
