#ifndef GEOIMAGELAYER_H
#define GEOIMAGELAYER_H

// Qt
#include <QObject>
#include <QString>
#include <QRect>
#include <QRectF>
#include <QPolygonF>

// Project
#include "Global.h"
#include "BaseLayer.h"

namespace Core
{

//******************************************************************************

class GeoShapeLayer : public BaseLayer
{

    Q_OBJECT

    // Geo info
    Q_PROPERTY(QRect pixelExtent READ getPixelExtent)
    PROPERTY_ACCESSORS(QRect, pixelExtent, getPixelExtent, setPixelExtent)
    Q_PROPERTY(QPolygonF geoExtent READ getGeoExtent)
    PROPERTY_ACCESSORS(QPolygonF, geoExtent, getGeoExtent, setGeoExtent)
    Q_PROPERTY(QRectF geoBBox READ getGeoBBox)
    PROPERTY_ACCESSORS(QRectF, geoBBox, getGeoBBox, setGeoBBox)

    Q_CLASSINFO("pixelExtent","label:Pixel extent")
    Q_CLASSINFO("geoExtent","label:Geo extent")
    Q_CLASSINFO("geoBBox","label:Geo bounding box")

public:
    GeoShapeLayer(QObject * parent = 0) :
        BaseLayer(parent)
    {
        _type = "Geo Shape";
    }


};

//******************************************************************************

class GeoImageLayer : public GeoShapeLayer
{
    Q_OBJECT

    // Image file info :
    Q_PROPERTY_WITH_ACCESSORS(QString, imageName, getImageName, setImageName)
//    Q_PROPERTY(QString filePath READ getFilePath)
//    PROPERTY_ACCESSORS(QString, filePath, getFilePath, setFilePath)
//    Q_PROPERTY(QString fileToOpen READ getFileToOpen)
//    PROPERTY_ACCESSORS(QString, fileToOpen, getFileToOpen, setFileToOpen)

    Q_CLASSINFO("imageName","label:Image name")
//    Q_CLASSINFO("filePath","label:File path")

    // Image/Geo info :
    Q_PROPERTY(int nbBands READ getNbBands)
    PROPERTY_ACCESSORS(int, nbBands, getNbBands, setNbBands)
    Q_PROPERTY(int depth READ getDepthInBytes)
    PROPERTY_ACCESSORS(int, depth, getDepthInBytes, setDepthInBytes)
    Q_PROPERTY(bool isComplex READ isComplex)
    PROPERTY_ACCESSORS(bool, isComplex, isComplex, setIsComplex)
    Q_PROPERTY(QString projectionRef READ getProjectionRef)
    PROPERTY_ACCESSORS(QString, projectionRef, getProjectionRef, setProjectionRef)

    Q_CLASSINFO("nbBands","label:Number of channels")
    Q_CLASSINFO("depth","label:Encoding")
    Q_CLASSINFO("isComplex","label:Complex pixel")
    Q_CLASSINFO("projectionRef","label:Projection reference")

public:
    explicit GeoImageLayer(QObject *parent = 0);

};

//******************************************************************************

}


#endif // GEOIMAGELAYER_H
