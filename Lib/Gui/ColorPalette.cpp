// Qt
#include <QGraphicsScene>
#include <QGraphicsSceneMoveEvent>
#include <QColorDialog>

// Project
#include "ColorPalette.h"
#include "Core/Global.h"


namespace Gui
{

//*************************************************************************

double tinyDelta=1e-9;

QGradientStops computeStopsFromValues(const QGradientStops & values, double hxmin, double hxmax)
{
    QGradientStops output = QGradientStops();
    foreach (QGradientStop value, values)
    {
        output << QGradientStop((value.first-hxmin)/(hxmax - hxmin), value.second);
    }
    return output;
}

QGradientStops computeValuesFromStops(const QGradientStops & stops, double hxmin, double hxmax)
{
    QGradientStops values = QGradientStops();
    foreach (QGradientStop stop, stops)
    {
        values << QGradientStop(stop.first*(hxmax-hxmin) + hxmin, stop.second);
    }
    return values;
}

//*************************************************************************

/*!

        \class ColorPalette
        \ingroup Gui
        \brief graphical representation of gradient color palette with slider

        \class Slider
        \ingroup Gui
        \brief graphical representation of the color palette slider

 */

//*************************************************************************

/*!
    Constructor
*/
ColorPalette::ColorPalette(QGraphicsItem * parent) :
    QGraphicsItem(parent),
    QObject(0),
    _colorPaletteRect(new QGraphicsRectItem(this)),
    _palette(0),
    _xmin(0.0),
    _xmax(1.0),
    _isDiscrete(false)
{
    _colorPaletteRect->setRect(0.0, 0.0, 1.0, _settings.paletteHeightRatio);
    _colorPaletteRect->setPen(QPen(Qt::black, 0.0));
}

QColor computeColorAtPosition(double position, const QGradientStop & leftStop, const QGradientStop & rightStop)
{ // Compute as linear interpolation between color of two stops

    double x = (position - leftStop.first)/(rightStop.first - leftStop.first);

    QColor output(
                leftStop.second.red()*(1-x) + x*rightStop.second.red(),
                leftStop.second.green()*(1-x) + x*rightStop.second.green(),
                leftStop.second.blue()*(1-x) + x*rightStop.second.blue()
                );
    return output;
}

//*************************************************************************

/*!
    Method to setup palette
    \param values are gradient stops on the color palette. Values can be normalized between 0.0 and 1.0
    \param valueMin is real that corresponds to normalized 0.0 value
    \param valueMax is real that corresponds to normalized 1.0 value
*/
void ColorPalette::setupPalette(const QGradientStops & normValues, double valueMin, double valueMax, bool isDiscrete)
{
    if(_palette)
        delete _palette;

    _xmin = valueMin;
    _xmax = valueMax;
    _stops = normValues;
    _isDiscrete = isDiscrete;

    _palette = new QLinearGradient(QPointF(0.0, 0.0), QPointF(1.0, 0.0));
    updateAllStops();

    // remove previous sliders
    foreach (Slider * s, _sliders)
    {
        scene()->removeItem(s);
    }
    _sliders.clear();

    int count=0;
    foreach (QGradientStop s, _stops)
    {
        /*Slider * slider = */createSlider(s.first, s.second, count);
        count++;

    }
    _colorPaletteRect->installSceneEventFilter(this);

}

//*************************************************************************

/*!
    Method to compute the real value of the slider \param index
*/
double ColorPalette::getValue(int index) const
{
    if (index < 0 || index > _stops.size()-1)
        return -12345;
    return _stops[index].first*(_xmax - _xmin) + _xmin;
}

//*************************************************************************

/*!
    Method to compute slider value
*/
double ColorPalette::getNormValue(int index) const
{
    if (index < 0 || index > _stops.size()-1)
        return -12345;
    return _stops[index].first;
}


//*************************************************************************

/*!
    Method to get slider normalized values with colors
*/
QGradientStops ColorPalette::getPalette() const
{
    if (!_palette)
        return QGradientStops();
//    return computeValuesFromStops(_stops, _xmin, _xmax);
    return _stops;

}

//*************************************************************************

/*!
    Method to update palette colors
*/
void ColorPalette::updateAllStops()
{

    QGradientStops nstops;
    if (_isDiscrete)
    { // add stops to palette only : [(s1), (s2), (s3),...] -> [(s1),(s11,s2),(s22,s3),...]
        nstops.resize(_stops.size()*2-1);
        nstops.first() = _stops.first();
        for (int i=1; i<_stops.size();i++)
        {
            nstops[2*i-1] = QGradientStop(_stops[i].first-tinyDelta,_stops[i-1].second);
            nstops[2*i] = _stops[i];
        }
    }
    else
    {// remove stops from palette only : [(s1),(s11,s2),(s22,s3),...] -> [(s1), (s2), (s3),...]
        nstops = _stops;
    }
    _palette->setStops(nstops);
    _colorPaletteRect->setBrush(*_palette);
}

//*************************************************************************

/*!
    Method to insert a new stop to the palette
*/
void ColorPalette::insertStop(int index, const QGradientStop &stop)
{
    _stops.insert(index, stop);
    if (!_isDiscrete)
    {
        _palette->setStops(_stops);
    }
    else
    {
        QGradientStops stops = _palette->stops();

        if (index > 0 && 2*index < stops.size())
        {
            QGradientStop prevStop = stops[2*(index-1)];
            stops[2*index-1].second = stop.second;
            QGradientStop nstop(stop.first-tinyDelta, prevStop.second);
            stops.insert(2*index - 1, nstop);
            stops.insert(2*index, stop);
        }
        else if (index == 0)
        {
            QGradientStop zeroStop = stops[0];
            QGradientStop nstop(zeroStop.first-tinyDelta, stop.second);
            stops.insert(0, nstop);
            stops.insert(0, stop);
        }
        else if (2*index >= stops.size())
        {
            QGradientStop lastStop = stops.last();
            QGradientStop nstop(stop.first-tinyDelta, lastStop.second);
            stops.insert(stops.size(), nstop);
            stops.insert(stops.size(), stop);
        }



        _palette->setStops(stops);
    }
    _colorPaletteRect->setBrush(*_palette);
}

//*************************************************************************

/*!
    Method to remove a stop to the palette
*/
void ColorPalette::removeStop(int index)
{
    _stops.remove(index);
    if (!_isDiscrete)
    {
        _palette->setStops(_stops);
    }
    else
    {
        QGradientStops stops = _palette->stops();

        if (index > 0 && 2*index < stops.size())
        {
            stops[2*(index+1)-1].second = stops[2*(index-1)].second;
            stops.remove(2*index-1, 2);
        }
        else if (index == 0)
        {
            stops.remove(0, 2);
        }
        else if (2*index == stops.size() - 1 )
        {
            stops.remove(stops.size()-2, 2);
        }
        _palette->setStops(stops);
    }
    _colorPaletteRect->setBrush(*_palette);

}

//*************************************************************************

/*!
    Method to modify the stop at the index
*/
void ColorPalette::modifyStop(int index, const QGradientStop & stop)
{
    _stops[index] = stop;
    if (!_isDiscrete)
    {
        _palette->setStops(_stops);
    }
    else
    {
        QGradientStops stops = _palette->stops();
        stops[2*index] = stop;
        if (index > 0)
            stops[2*index-1].first = stop.first - tinyDelta;
        if (2*index < stops.size()-1)
        {
            stops[2*(index+1)-1].second = stop.second;
        }
        _palette->setStops(stops);
    }
    _colorPaletteRect->setBrush(*_palette);
}

//*************************************************************************

///*!
//    Method to set new min/max limits and recompute inner slider positions
//*/
//void ColorPalette::setMinMaxRanges(double xmin, double xmax)
//{
//    double oldXMin(_xmin), oldXMax(_xmax);
//    for (int i=0;i<_sliders.size();i++)
//    {
//        Slider * s = _sliders[i];

//        QPointF pos = s->pos();
//        double x = (oldXMin - xmin)/(xmax - xmin) + pos.x()*(oldXMax - oldXMin)/(xmax - xmin);
//        x = (x > 1.0) ? 1.0 : (x < 0.0) ? 0.0 : x;
//        pos.setX(x);

//        s->setFlag(ItemSendsGeometryChanges, false);
//        s->setPos(pos);
//        s->setFlag(ItemSendsGeometryChanges, true);

//        // update gradient:
//        QGradientStop & currentStop = _stops[i];
//        currentStop.first = s->x();
//    }
//    updateAllStops();
//    _xmin = xmin;
//    _xmax = xmax;

//}

//*************************************************************************

/*!
    Method to create a slider
*/
Slider * ColorPalette::createSlider(double xpos, const QColor & color, int count)
{
    Slider * slider = new Slider(this);
    _sliders.insert(count, slider);

    slider->setZValue(1.0);
    slider->setColor(color);
	slider->setScale(0.06);
    slider->setupProperties(0.0, 1.0, _settings.paletteHeightRatio);
    slider->installSceneEventFilter(this);
    slider->setText(QString("%1").arg(xpos*(_xmax - _xmin) + _xmin));
    slider->setFlag(ItemSendsGeometryChanges,false);
    slider->setPos(xpos,_settings.paletteHeightRatio);
    slider->setFlag(ItemSendsGeometryChanges,true);
    return slider;
}

//*************************************************************************

/*!
    Method to add slider at position
*/
bool ColorPalette::addSlider(const QPointF &position, int * index)
{
    double x = position.x();
    // find index to where to insert it
    int count=0;
    foreach (Slider* slider, _sliders)
    {
        if (slider->pos().x() > x)
        {
            break;
        }
        count++;
    }
    // create new color stop
    QColor c;
    if (count > 0 && count < _stops.size() )
    {
        c = computeColorAtPosition(x, _stops[count-1], _stops[count]);
    }
    else if (count == 0)
    {
        c = _stops[count].second;
    }
    else if (count == _stops.size())
    {
        c = _stops[count - 1].second;
    }

    // create new slider
    /*Slider * newSlider = */createSlider(x, c, count);


    QGradientStop newStop(x, c);
    insertStop(count, newStop);

    // store index
    if (index)
        *index=count;

    return true;
}

//*************************************************************************

/*!
    Method to remove slider
*/
bool ColorPalette::removeSliderAtIndex(int index)
{
    // remove slider:
    if (index < 0 || index > _sliders.size() - 1)
    {
        SD_TRACE("ColorPalette::removeSlider : index is out of bounds");
        return false;
    }

    scene()->removeItem(_sliders[index]);
    // -> The ownership of item is passed on to the caller
    delete _sliders[index];
    _sliders.removeAt(index);

    // update gradient:
    removeStop(index);

    return true;
}

//*************************************************************************

/*!
    Method to set new (normalized) value of slider at index
*/
void ColorPalette::setSliderValueAtIndex(int index, double value)
{
    if (index < 0 || index > _sliders.size() - 1)
    {
        SD_TRACE("ColorPalette::setColorOfSliderAtIndex : index is out of bounds");
        return;
    }
    Slider * s=_sliders[index];
    QPointF pos = s->pos();
//    double x = (value-_xmin)/(_xmax - _xmin);
    double x = value;
    x = (x > 1.0) ? 1.0 : (x < 0.0) ? 0.0 : x;
    pos.setX(x);
    s->setPos(pos);
}

//*************************************************************************

/*!
    Method to set color of slider at index
*/
void ColorPalette::setColorOfSliderAtIndex(int index, const QColor &c)
{
    if (index < 0 || index > _sliders.size() - 1)
    {
        SD_TRACE("ColorPalette::setColorOfSliderAtIndex : index is out of bounds");
        return;
    }
    _sliders[index]->setColor(c);

    // update gradient:
    QGradientStop nstop(_stops[index].first, c);
    modifyStop(index, nstop);

}

//*************************************************************************

/*!
    Method to reset color of slider at index
*/
void ColorPalette::resetColorOfSliderAtIndex(int index)
{
    if (index < 0 || index > _sliders.size() - 1)
    {
        SD_TRACE("ColorPalette::setColorOfSliderAtIndex : index is out of bounds");
        return;
    }
    // compute default color
    QColor c;
    double x = _sliders[index]->pos().x();
    if (index > 0 && index < _stops.size() - 1 )
    {
        c = computeColorAtPosition(x, _stops[index-1], _stops[index+1]);
    }
    else if (index == 0)
    {
        c = Qt::black;
    }
    else if (index == _stops.size()-1)
    {
        c = Qt::white;
    }

    _sliders[index]->setColor(c);
    // update gradient:
    QGradientStop nstop(_stops[index].first, c);
    modifyStop(index, nstop);

}

//*************************************************************************

/*!
    Method to highlight slider text at index
*/
void ColorPalette::highlightSliderTextAtIndex(int index, bool value)
{
    if (index < 0 || index > _sliders.size() - 1)
    {
        SD_TRACE("ColorPalette::setColorOfSliderAtIndex : index is out of bounds");
        return;
    }
    _sliders[index]->highlightText(value);
}

//*************************************************************************

/*!
    Method to set color palette mode to discrete
*/
void ColorPalette::setIsDiscrete(bool v)
{
    _isDiscrete = v;
    updateAllStops();
}

//*************************************************************************

/*!
    \overload
*/
QRectF ColorPalette::boundingRect() const
{
    return QRectF(0.0, 0.0, 1.0, 1.0);
}

//*************************************************************************

/*!
    empty method
*/
void ColorPalette::paint(QPainter * /*p*/, const QStyleOptionGraphicsItem * /*o*/, QWidget * /*w*/)
{
}

//*************************************************************************

/*!
    Method to update colors
*/
void fixPositionWithRightBoundary(QPointF * p, const QPointF & b)
{
    if (b.x() - p->x() <= 0 )
    {
        p->setX(b.x() - 1e-5);
    }
}

void fixPositionWithLeftBoundary(QPointF * p, const QPointF & b)
{
    if (b.x() - p->x() >= 0 )
    {
        p->setX(b.x() + 1e-5);
    }
}

void ColorPalette::preventCollisionsAndUpdateGradient(Slider *slider, QPointF * csPos)
{
    if (!(_palette && _sliders.size() == _stops.size()))
        return;

    // prevent collisions:
    int sliderIndex = _sliders.indexOf(slider);
    if (sliderIndex < 0)
    { // typical situation when setPos() is called on a new slider, but it is not yet in the list of sliders

        SD_TRACE("ColorPalette::preventCollisionsAndUpdateGradient : slider is not found");
        return;
    }

    if (sliderIndex >= 0 && sliderIndex < _sliders.size() - 1)
    { // Check only right neighbour
        Slider * rightSlider = _sliders[sliderIndex+1];
        fixPositionWithRightBoundary(csPos, rightSlider->pos());
    }

    if (sliderIndex <= _sliders.size() - 1 && sliderIndex > 0)
    { // Check only left neighbour
        Slider * leftSlider = _sliders[sliderIndex-1];
        fixPositionWithLeftBoundary(csPos, leftSlider->pos());
    }

    // update gradient:
    QGradientStop newStop(csPos->x(), _stops[sliderIndex].second);
    modifyStop(sliderIndex, newStop);

    // Update text:
    slider->setText(QString("%1").arg(csPos->x()*(_xmax-_xmin) + _xmin));

    // notify about the change
//    emit sliderPositionChanged(sliderIndex, csPos->x()*(_xmax-_xmin) + _xmin);
    emit sliderPositionChanged(sliderIndex, csPos->x());

}

//*************************************************************************
//*************************************************************************
//*************************************************************************

/*!
    \overload
    - restrict items position
    - prevent collisions and update gradient color
*/
QVariant Slider::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene())
    {
        // Limit position to the Slider available line
        QPointF newPos = value.toPointF();
        if (qAbs(newPos.y() - _fixedValue) > 1e-5 ||
                newPos.x() > _motionRangeMax ||
                newPos.x() < _motionRangeMin)
        {
            newPos.setX(qMin(_motionRangeMax, qMax(_motionRangeMin, newPos.x())));
            newPos.setY(_fixedValue);
        }

        // Prevent sliders collisions <-> Slider x positions are ordered and can not be identical
        if (_parent)
            _parent->preventCollisionsAndUpdateGradient(this, &newPos);
        return newPos;
    }
    return QGraphicsItem::itemChange(change, value);
}

//*************************************************************************

/*!
  Method to set brush
 */
void Slider::setColor(const QColor &c)
{
    setBrush(c);
    if (c == Qt::black)
        setPen(QPen(Qt::white, 0.0));
    else
        setPen(QPen(Qt::black, 0.0));

}

//*************************************************************************

/*!
    \overload
  */
void Slider::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

     // keep text unscaled
    QTransform tr = painter->transform();
    _text->setTransform(
                QTransform::fromTranslate( _size , 2.0 * _size) *
                QTransform::fromScale(1.0/tr.m11(), 1.0/tr.m22())

                );

//    std::cout << "Tr : " << tr.m11() << ", " << tr.m22() << std::endl;
//    std::cout << "Widget : " << widget->width() << ", " << widget->height() << std::endl;

    QRectF r = _poly.boundingRect();
    double f = 0.1;
    r.adjust(-f*r.width(), 0.0, f*r.width(), 5.0*f*r.height());
    _brect.setX(r.x() * _size / tr.m11());
    _brect.setY(r.y() * _size / tr.m22());
    _brect.setWidth(r.width() * _size / tr.m11());
    _brect.setHeight(r.height() * _size / tr.m22());

//    painter->setBrush(QColor(0,255,0,127));
//    painter->setPen(QPen(QColor(Qt::black), 0.0));
//    painter->drawRect(boundingRect());

//    painter->setBrush(QColor(255,0,0,127));
//    painter->setPen(QPen(QColor(Qt::black), 0.0));
//    painter->drawRect(_brect);


    QTransform ntr = QTransform::fromScale(_size, _size) * QTransform::fromTranslate(tr.m31(), tr.m32());
    painter->setTransform(ntr);

    QGraphicsPolygonItem::paint(painter, option, widget);

}

//*************************************************************************

/*!
    \overload
*/
QRectF Slider::boundingRect() const
{
    return _brect;
}

//*************************************************************************

/*!
    \overload
*/
QPainterPath Slider::shape() const
{
    QPainterPath path;
    path.addRect(_brect);
    return path;
}

//*************************************************************************

} // namespace Gui

