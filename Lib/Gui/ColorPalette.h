#ifndef COLORPALETTE_H
#define COLORPALETTE_H


// Qt
#include <QGraphicsObject>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QLinearGradient>
#include <QMap>
#include <QMenu>
#include <QColorDialog>
#include <QGraphicsScene>

// Project
#include "Core/LibExport.h"

namespace Gui
{

//*************************************************************************

class Slider;

class GIV_DLL_EXPORT ColorPalette : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:

    friend class Slider;

    struct Settings
    {
        double paletteHeightRatio;

        Settings() :
            paletteHeightRatio(0.2)
        {
        }
    };



public:
    enum { Type = UserType + 1 };
    virtual int type() const { return Type; }

    explicit ColorPalette(QGraphicsItem * parent = 0);

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *o, QWidget * w);

    void setupPalette(const QGradientStops & values, double valueMin, double valueMax, bool isDiscrete);
    void setMinMaxRanges(double xmin, double xmax);
    QPair<double, double> getMinMaxRanges() const
    { return QPair<double, double>(_xmin, _xmax); }


    QGradientStops getPalette() const;
    double getValue(int index) const;

    bool itemIsSlider(QGraphicsItem* item) const
    { return _sliders.contains(reinterpret_cast<Slider*>(item)); }

    bool itemIsPalette(QGraphicsItem* item) const
    { return _colorPaletteRect == item; }

    bool itemIsSliderText(QGraphicsItem* simpletextitem) const
    { return qgraphicsitem_cast<QGraphicsSimpleTextItem*>(simpletextitem) != 0; }

    void highlightSliderTextAtIndex(int index, bool value=true);

    int getNbOfSliders() const
    { return _sliders.size(); }

    int getSliderIndex(QGraphicsItem* slider) const
    { return _sliders.indexOf(reinterpret_cast<Slider*>(slider)); }

    bool removeSliderAtIndex(int sliderIndex);
    bool addSlider(const QPointF & position, int *index = 0);

    void setSliderValueAtIndex(int index, double value);

    void setColorOfSliderAtIndex(int index, const QColor & c);
    void resetColorOfSliderAtIndex(int index);

    bool isDiscrete() const
    { return _isDiscrete; }
    void setIsDiscrete(bool v);

protected:
    void preventCollisionsAndUpdateGradient(Slider * slider, QPointF * p);

    void insertStop(int index, const QGradientStop & stop);
    void removeStop(int index);
    void modifyStop(int index, const QGradientStop & stop);
    void updateAllStops();


signals:
    void sliderPositionChanged(int index, double position);

private:

    Slider * createSlider(double xpos, const QColor & color, int count);

    QGraphicsRectItem *_colorPaletteRect;
    QLinearGradient * _palette;
    QList<Slider*> _sliders;
    QGradientStops _stops; //! User selected stops. They can be different from real _palette stops (due to discrete color option)
    Settings _settings;

    double _xmin;
    double _xmax;

    bool _isDiscrete;
};

//*************************************************************************

class Slider : public QGraphicsPolygonItem
{
public:
    explicit Slider(ColorPalette *parent = 0) :
        QGraphicsPolygonItem(parent),
        _parent(parent),
        _size(5.0),
        _motionRangeMin(0.),
        _motionRangeMax(1.),
        _fixedValue(0.5)
    {

        _poly = QPolygonF() << QPointF(0, -1.0) <<
                               QPointF(-1.732, 0.5) <<
                               QPointF(1.732, 0.5);

        _brect = _poly.boundingRect();
        setPolygon(_poly);
		
        setBrush(Qt::white);
        setFlag(ItemIsMovable);
        setFlag(ItemSendsGeometryChanges);
        setAcceptedMouseButtons((Qt::MouseButtons)(Qt::LeftButton | Qt::RightButton));


        _text = new QGraphicsSimpleTextItem;
        _text->setParentItem(this);
        _text->setPen(QPen(Qt::black, 0.0));
        _text->setRotation(90.0);
        _text->installSceneEventFilter(this);

        QFont f;
        f.setPixelSize(10);
        f.setStyleHint(QFont::System);
        f.setWeight(QFont::Light);
        _text->setFont(f);
    }

    void setupProperties(double rangeMin, double rangeMax, double fixedValue)
    {
        _motionRangeMin = rangeMin;
        _motionRangeMax = rangeMax;
        _fixedValue = fixedValue;
    }

    void setText(const QString & text)
    { _text->setText(text); }

    void setColor(const QColor & c);

    void highlightText(bool value)
    { value ? _text->setPen(QPen(Qt::red, 0.0)) : _text->setPen(QPen(Qt::black, 0.0)); }

    virtual QPainterPath shape() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    double _size;
    double _motionRangeMin;
    double _motionRangeMax;
    double _fixedValue;
    ColorPalette * _parent;
    QGraphicsSimpleTextItem * _text;
    QPolygonF _poly;
    QRectF _brect;
};

//*************************************************************************

QGradientStops computeStopsFromValues(const QGradientStops & values, double hxmin, double hxmax);
QGradientStops computeValuesFromStops(const QGradientStops & stops, double hxmin, double hxmax);

}

#endif // COLORPALETTE_H

