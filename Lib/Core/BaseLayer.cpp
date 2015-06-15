
// Project
#include "BaseLayer.h"

namespace Core
{

//******************************************************************************


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
