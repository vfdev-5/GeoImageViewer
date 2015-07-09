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
#include "LibExport.h"
#include "GeoShapeLayer.h"

namespace Core
{

typedef QPair<QString, QString> MetadataItem;

//******************************************************************************

class GIV_DLL_EXPORT GeoImageLayer : public GeoShapeLayer
{
    Q_OBJECT

    // Image file info :
    Q_PROPERTY_WITH_ACCESSORS(QString, imageName, getImageName, setImageName)
    Q_CLASSINFO("imageName","label:Image name")

    Q_PROPERTY(QString location READ getLocation)
    PROPERTY_ACCESSORS(QString, location, getLocation, setLocation)

    // Image/Geo info :
    Q_PROPERTY(QRect pixelExtent READ getPixelExtent)
    PROPERTY_ACCESSORS(QRect, pixelExtent, getPixelExtent, setPixelExtent)

    Q_PROPERTY(int nbBands READ getNbBands)
    PROPERTY_ACCESSORS(int, nbBands, getNbBands, setNbBands)

    Q_PROPERTY(int depth READ getDepthInBytes)
    PROPERTY_ACCESSORS(int, depth, getDepthInBytes, setDepthInBytes)

    Q_PROPERTY(bool isComplex READ isComplex)
    PROPERTY_ACCESSORS(bool, isComplex, isComplex, setIsComplex)

    Q_CLASSINFO("pixelExtent","label:Pixel extent")
    Q_CLASSINFO("nbBands","label:Number of channels")
    Q_CLASSINFO("depth","label:Encoding (bytes)")
    Q_CLASSINFO("isComplex","label:Complex pixel")

//    Q_PROPERTY(double noDataValue READ getNoDataValue)
//    PROPERTY_ACCESSORS(double, noDataValue, getNoDataValue, setNoDataValue)
//    Q_CLASSINFO("noDataValue","label:NoData Value")

    Q_PROPERTY(QString projectionRef READ getProjectionRef)
    PROPERTY_ACCESSORS(QString, projectionRef, getProjectionRef, setProjectionRef)
    Q_PROPERTY(QVector<double> geoTransform READ getGeoTransform)
    PROPERTY_ACCESSORS(QVector<double>, geoTransform, getGeoTransform, setGeoTransform)
//    Q_PROPERTY(QList<MetadataItem> metadata READ getMetadata)
    PROPERTY_ACCESSORS(QList< MetadataItem >, metadata, getMetadata, setMetadata)

    Q_CLASSINFO("projectionRef","label:Projection reference")
    Q_CLASSINFO("geoTransform","label:Geo transform")
    Q_CLASSINFO("metadata","label:Image metadata")

public:
    explicit GeoImageLayer(QObject *parent = 0);

    virtual bool canSave()
    { return true; }

    void addMetadataItem(const QString & name, const QString & data)
    { _metadata.append(MetadataItem(name, data)); }


};

//******************************************************************************

}


#endif // GEOIMAGELAYER_H
