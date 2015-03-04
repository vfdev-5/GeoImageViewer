#ifndef BASELAYER_H
#define BASELAYER_H

// Qt
#include <QObject>

// Project
#include "Global.h"

namespace Core
{

//******************************************************************************


class BaseLayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isVisible READ isVisible WRITE setVisible)
    PROPERTY_GETACCESSOR(bool, isVisible, isVisible)
    Q_PROPERTY(double opacity READ getOpacity WRITE setOpacity)
    PROPERTY_GETACCESSOR(double, opacity, getOpacity)

    PROPERTY_ACCESSORS(QString, type, getType, setType)
    PROPERTY_GETACCESSOR(int, zValue, getZValue)

    Q_CLASSINFO("isVisible","label:Visible")
    Q_CLASSINFO("opacity","label:Transparency;minValue:0.0;maxValue:1.0")

public:
    explicit BaseLayer(QObject *parent = 0);

    void setVisible(bool visible);
    void setOpacity(double opacity);
    void setZValue(int zValue);

    virtual bool canSave()
    { return false; }

signals:
    void layerStateChanged();

public slots:

};

//******************************************************************************

}

#endif // BASELAYER_H
