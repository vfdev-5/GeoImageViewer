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
#include <QGraphicsSceneContextMenuEvent>
#include <QLineEdit>

// Project
#include "Core/LibExport.h"
#include "ColorPickerFrame.h"

namespace Gui
{

//*************************************************************************

class Slider;

//class GIV_DLL_EXPORT ColorPalette : public QObject, public QGraphicsItem
class GIV_DLL_EXPORT ColorPalette : public QGraphicsObject
{
    Q_OBJECT
public:

    friend class Slider;

    struct Settings
    {
        double paletteHeightRatio;
        bool editable;
        bool showValues;

        Settings() :
            paletteHeightRatio(0.2),
            editable(true),
            showValues(true)
        {
        }
    };



public:
    enum { Type = UserType + 1001 };
    int type() const
    { return Type; }

    explicit ColorPalette(QGraphicsItem * parent = 0);
    virtual ~ColorPalette();

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *o, QWidget * w);

    void setupPalette(const QGradientStops & normValues, double valueMin, double valueMax, bool isDiscrete);

    QPair<double, double> getMinMaxRanges() const
    { return QPair<double, double>(_xmin, _xmax); }

    QGradientStops getPalette() const;
    double getValue(int index) const;
    double getNormValue(int index) const;

    static bool itemIsPalette(QGraphicsItem* item)
    { return qgraphicsitem_cast<QGraphicsRectItem*>(item) &&
                qgraphicsitem_cast<ColorPalette*>(item->parentItem());  }

    static bool itemIsSliderText(QGraphicsItem* simpletextitem)
    { return qgraphicsitem_cast<QGraphicsSimpleTextItem*>(simpletextitem) != 0 &&
                qgraphicsitem_cast<Slider*>(simpletextitem->parentItem()) != 0; }

    void highlightSliderTextAtIndex(int index, bool value=true);

    int getNbOfSliders() const
    { return _sliders.size(); }

    int getSliderIndex(Slider* slider) const
    { return _sliders.indexOf(slider); }

    Slider * getSlider(int index)
    {
        if (index < 0 || index >= _sliders.size())
            return 0;
        return _sliders[index];
    }

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


    bool eventFilter(QObject *, QEvent *);
    void valueEditorEvents(QEvent * event);
    bool sceneEventFilter(QGraphicsItem * watched, QEvent * event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

protected slots:
    void onMenuTriggered(QAction * );
    void onColorPicked(QColor c);
    void onValueEdited();

signals:
    void sliderAdded(Slider *);
    void sliderRemoved();
    void sliderColorChanged(Slider * slider, const QColor & c);
    void sliderPositionChanged(Slider * slider, double position);
//    void sliderMouseRelease(Slider * slider, double position);

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

    bool _sliderPressed;
    bool _sliderMoving;

    QMenu _menu;
    QAction _removeSlider;
    QAction _addSlider;
    QAction _revertSlider;

    Slider * _actionedSlider;

    ColorPickerFrame * _colorPicker;
    QLineEdit * _valueEditor;


};

//*************************************************************************

class Slider : public QGraphicsPolygonItem
{
public:
    explicit Slider(ColorPalette *parent = 0) :
        QGraphicsPolygonItem(parent),
        _parent(parent),
        _size(6.0),
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
        _text->installSceneEventFilter(parent);

        QFont f;
        f.setPixelSize(10);
        f.setStyleHint(QFont::System);
        f.setWeight(QFont::Light);
        _text->setFont(f);
    }

    virtual ~Slider()
    {
        delete _text;
    }

    enum { Type = UserType + 1002 };
    int type() const
    { return Type; }

    void setupProperties(double rangeMin, double rangeMax, double fixedValue)
    {
        _motionRangeMin = rangeMin;
        _motionRangeMax = rangeMax;
        _fixedValue = fixedValue;
    }

    void setTextVisible(bool value)
    { _text->setVisible(value); }

    void setText(const QString & text)
    { _text->setText(text); }

    void setColor(const QColor & c);

    void highlightText(bool value)
    { value ? _text->setPen(QPen(Qt::red, 0.0)) : _text->setPen(QPen(Qt::black, 0.0)); }

    virtual QRectF boundingRect() const;
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

