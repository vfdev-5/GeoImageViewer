
// Project
#include "GeoImageLayer.h"

namespace Core
{

//******************************************************************************
/*!
  \class GeoImageLayer
  \brief derived from GeoShapeLayer and represents geographic image layers.
  It has properties :

    - image name
    - pixel extent
    - number of bands
    - single band pixel depth
    - if is complex

    - projection string
    - geo transform
    - metadata
  */

//******************************************************************************

GeoImageLayer::GeoImageLayer(QGraphicsItem *item, QObject *parent) :
    GeoShapeLayer(item, parent)
{
    _type = "Geo Image";
}

//******************************************************************************

}
