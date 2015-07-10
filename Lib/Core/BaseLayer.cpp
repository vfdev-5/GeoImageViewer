
// Project
#include "BaseLayer.h"

namespace Core
{

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
 */

BaseLayer::BaseLayer(QObject *parent) :
    QObject(parent),
    _isVisible(true),
    _opacity(1.0),
    _zValue(0),
    _editable(false)
{
}

//******************************************************************************

void BaseLayer::setVisible(bool visible)
{
    if (_isVisible != visible)
    {
        _isVisible = visible;
        emit layerStateChanged();
    }
}

//******************************************************************************

void BaseLayer::setOpacity(double opacity)
{
    if (_opacity != opacity)
    {
        _opacity = opacity;
        emit layerStateChanged();
    }
}

//******************************************************************************

void BaseLayer::setZValue(int zValue)
{
    if (_zValue != zValue)
    {
        _zValue = zValue;
        emit layerStateChanged();
    }
}

//******************************************************************************

}
