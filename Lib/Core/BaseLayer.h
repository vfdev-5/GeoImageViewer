#ifndef BASELAYER_H
#define BASELAYER_H

// Qt
#include <QObject>

// Project
#include "Global.h"
#include "LibExport.h"

class QGraphicsItem;

namespace Core
{

//******************************************************************************
/*!
  \class BaseLayer
  \brief base structure used to represent graphics item of the ShapeViewer.
  It has properties:
    - if layer is visible
    - opacity
    - type of layer
    - z value
    - if can be saved

  It has (owns) as attribute associated graphics item instance.

 */

//******************************************************************************

class GIV_DLL_EXPORT BaseLayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isVisible READ isVisible WRITE setVisible)
    PROPERTY_GETACCESSOR(bool, isVisible, isVisible)
    Q_PROPERTY(double opacity READ getOpacity WRITE setOpacity)
    PROPERTY_GETACCESSOR(double, opacity, getOpacity)

    Q_PROPERTY(QString type READ getType WRITE setType)
    PROPERTY_ACCESSORS(QString, type, getType, setType)

    Q_PROPERTY(double zValue READ getZValue WRITE setZValue)
    PROPERTY_GETACCESSOR(double, zValue, getZValue)
    PROPERTY_ACCESSORS(bool, editable, isEditable, setEditable)

    Q_CLASSINFO("isVisible","label:Visible")
    Q_CLASSINFO("opacity","label:Transparency;minValue:0.0;maxValue:1.0")

public:
    explicit BaseLayer(QGraphicsItem * item, QObject *parent = 0);
    virtual ~BaseLayer();

    void setVisible(bool visible);
    void setOpacity(double opacity);
    void setZValue(double zValue);

    const QGraphicsItem * getConstItem() const
    { return _item; }

    QGraphicsItem * getItem()
    { return _item; }

    virtual bool canSave()
    { return false; }

//signals:
//    void layerStateChanged();

public slots:

protected:
    QGraphicsItem * _item;

};

//******************************************************************************

double GIV_DLL_EXPORT computeZValue(int parentZ, int row, int level);

}

#endif // BASELAYER_H
