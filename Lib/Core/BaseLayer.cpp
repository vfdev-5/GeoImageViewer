
// Qt
#include <QGraphicsItem>
#include <QGraphicsScene>

// Project
#include "BaseLayer.h"

namespace Core
{

double computeZValue(int index)
{
    return 10.0 + 2.0 * index;
}

//******************************************************************************

/*!
 * \class BaseLayer
 * \brief Base structure to represent layer on the scene.
 * Properties :
 *  - type (text) denotes layer type
 *  - isVisible (bool) indicates if layer is visible
 *  - opacity ([0.0,1.0]) denotes layer's transparency
 *  - zValue (double) denotes visible order of the layer on the scene
 *  - editable (bool) indicates if layer can be editable with a tool
 *
 * Attribute :
 *  - QGraphicsItem is the associated graphics item
 */

BaseLayer::BaseLayer(QGraphicsItem *item, QObject *parent) :
    QObject(parent),
    _isVisible(true),
    _opacity(1.0),
    _zValue(0),
    _editable(false),
    _item(item)
{
}

//******************************************************************************

BaseLayer::~BaseLayer()
{
    if (_item)
    {
        QGraphicsScene * scene = _item->scene();
        if (scene)
        {
            scene->removeItem(_item);
        }
        delete _item;
    }
}

//******************************************************************************

void BaseLayer::setVisible(bool visible)
{
    if (_isVisible != visible)
    {
        _isVisible = visible;
        _item->setVisible(_isVisible);
//        emit layerStateChanged();
    }
}

//******************************************************************************

void BaseLayer::setOpacity(double opacity)
{
    if (_opacity != opacity)
    {
        _opacity = opacity;
        _item->setOpacity(_opacity);
//        emit layerStateChanged();
    }
}

//******************************************************************************

void BaseLayer::setZValue(int zValue)
{
    if (_zValue != zValue)
    {
        _zValue = zValue;
        _item->setZValue(computeZValue(_zValue));
//        emit layerStateChanged();
    }
}

//******************************************************************************

}
