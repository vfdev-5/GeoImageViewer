#ifndef IMAGE_H
#define IMAGE_H

// Qt
#include <QObject>
#include <QString>
#include <QRect>
#include <QRectF>
#include <QPolygonF>

// Project
#include "Global.h"

namespace Core
{

//******************************************************************************

class Image : public QObject
{

    Q_OBJECT
    // Image file info :
    Q_PROPERTY(QString filePath READ getFilePath)
    PROPERTY_ACCESSORS(QString, filePath, getFilePath, setFilePath)
    Q_PROPERTY(QString fileToOpen READ getFileToOpen)
    PROPERTY_ACCESSORS(QString, fileToOpen, getFileToOpen, setFileToOpen)
    Q_PROPERTY_WITH_ACCESSORS(QString, imageName, getImageName, setImageName)

    // Image/Geo info :
    Q_PROPERTY(int nbBands READ getNbBands)
    PROPERTY_ACCESSORS(int, nbBands, getNbBands, setNbBands)
    Q_PROPERTY(int depth READ getDepthInBytes)
    PROPERTY_ACCESSORS(int, depth, getDepthInBytes, setDepthInBytes)
    Q_PROPERTY(bool isComplex READ isComplex)
    PROPERTY_ACCESSORS(bool, isComplex, isComplex, setIsComplex)
    Q_PROPERTY(QRect pixelExtent READ getPixelExtent)
    PROPERTY_ACCESSORS(QRect, pixelExtent, getPixelExtent, setPixelExtent)
    Q_PROPERTY(QPolygonF geoExtent READ getGeoExtent)
    PROPERTY_ACCESSORS(QPolygonF, geoExtent, getGeoExtent, setGeoExtent)
    Q_PROPERTY(QRectF geoBBox READ getGeoBBox)
    PROPERTY_ACCESSORS(QRectF, geoBBox, getGeoBBox, setGeoBBox)
    Q_PROPERTY(QString projectionRef READ getProjectionRef)
    PROPERTY_ACCESSORS(QString, projectionRef, getProjectionRef, setProjectionRef)

public:
    Image();

protected:

};

//******************************************************************************

}

#endif // IMAGE_H
