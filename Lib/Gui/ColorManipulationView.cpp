
// Qt
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QContextMenuEvent>

// Project
#include "Core/Global.h"
#include "ColorManipulationView.h"


namespace std {

template<>
struct less<QGradientStop> : public std::binary_function<QGradientStop, QGradientStop, bool>
{
    bool
    operator()(const QGradientStop& __x, const QGradientStop& __y) const
    { return __x.first < __y.first; }
};

}

namespace Gui
{

//*************************************************************************

QGraphicsLineItem * createLine(QGraphicsItem * parent=0)
{
    QGraphicsLineItem * line = new QGraphicsLineItem(
                0.0,0.0,
                0.0,1.0,
                parent);
    line->setPen(QPen(QBrush(Qt::white), 0.0, Qt::DashLine));
    line->setZValue(2.0);
    return line;
}

QTransform sliderLineTransform(QGraphicsLineItem * sliderLine, QGraphicsItem * slider)
{
    return QTransform::fromTranslate(slider->scenePos().x(), 0.0) *
            sliderLine->sceneTransform().inverted();
}

inline void copyPositions(const QGradientStops & src, QGradientStops & dst)
{
    if (src.size() != dst.size()) return;
    for(int i=0;i<src.size();i++)
    {
        dst[i].first = src[i].first;
    }
}

void orderStops(QGradientStops * stops)
{
    qSort(stops->begin(), stops->end(), std::less<QGradientStop>());
}

void printStops(const QGradientStops & stops)
{
    SD_TRACE("----- Print stops -----");
    SD_TRACE(QString("Stops count : %1").arg(stops.size()));
    foreach (QGradientStop stop, stops)
    {
        SD_TRACE(QString("Stop : %1, %2").arg(stop.first).arg(stop.second.name()));
    }
    SD_TRACE("-----------------------");
}

//*************************************************************************

/*!

    \class ColorManipulationView
    \ingroup Gui
    \brief This class implements the histogram color manipulation widget. This is
        used to enhance color contrast and visualization using image histogram, pseudo-colors,
        color transfer function...

 */

//*************************************************************************

/*!
    Constructor
*/
ColorManipulationView::ColorManipulationView(QWidget *parent) :
    HistogramView(parent),
    _zoomFitSliders(tr("Fit sliders"), this)
//    _currentColorPalette(0)
{

    //    // setup timer:
    //    _updateDelayTimer.setSingleShot(true);
    //    connect(&_updateDelayTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerTimeout()));


    // setup context menu:
    setupContextMenu();

    _histogramScene.installEventFilter(this);

    _settings.histogramTransform =
            QTransform::fromScale(1.0,_cmvSettings.histOverPaletteRatio);

    _cmvSettings.colorPaletteTransform =
            QTransform::fromScale(1.0, 0.5*(1.0 -_cmvSettings.histOverPaletteRatio)) *
            QTransform::fromTranslate(0.0, (_histogramScene.height()-_settings.margin)*_cmvSettings.histOverPaletteRatio);

    _cmvSettings.colorPaletteTransform2 =
            QTransform::fromScale(1.0, 0.5*(1.0 -_cmvSettings.histOverPaletteRatio)) *
            QTransform::fromTranslate(0.0, (_histogramScene.height()-_settings.margin)*_cmvSettings.histOverPaletteRatio);


    // Draw items:
    // clear();

}

//*************************************************************************

/*!
    Method to clear all
*/
void ColorManipulationView::clear()
{
//    _colorPalette = 0;
//    _sliderLines.clear();
//    _currentColorPalette = 0;
    _colorPalettes.clear();
    HistogramView::clear();
//    createColorPalette();
}

//*************************************************************************

/*!
    Method to setup context menu
*/
void ColorManipulationView::setupContextMenu()
{
    // connect actions:
    connect(&_zoomFitSliders, SIGNAL(triggered()), this, SLOT(onFitSliders()));
}

//*************************************************************************

/*!
 * Method to add histogram data.
 * \param GradientStops are normalized color stops for the ColorPalette
 * \param QVector<double> data is normalized histogram values
*/
void ColorManipulationView::addHistogram(const QGradientStops & normStops, const QVector<double> & data, double xmin, double xmax, bool isDiscrete)
{
    int index = _histograms.size();
    _histograms.append(new CMVHistogramItem());

    if (!setHistogram(index, normStops, data, xmin, xmax, isDiscrete))
    {
        HistogramItem * h = _histograms.takeLast();
        delete h;
    }
}

//*************************************************************************

/*!
    Method to set histogram data at index.
*/
bool ColorManipulationView::setHistogram(int index, const QGradientStops & normStops, const QVector<double> & data, double xmin, double xmax, bool isDiscrete)
{
    if (index < 0 || index >= _histograms.size())
    {
        return false;
    }
    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_histograms[index]);
    if (!h)
        return false;

    HistogramView::setHistogram(index, data, xmin, xmax);
    h->isDiscrete=isDiscrete;
    h->outputStops=normStops;

    return true;
}

//*************************************************************************
/*!
    Method to draw histogram in gray or rgb mode :
*/
#define CheckIndex(index) (index < 0 || index >= _histograms.size())
void ColorManipulationView::drawSingleHistogram(int index)
{

    if (CheckIndex(index))
    {
        SD_TRACE("ColorManipulationView::drawSingleHistogram : index is out of bounds");
        return;
    }

    _cmvSettings.interactiveColorPalette = true;

    HistogramView::drawSingleHistogram(index);

    // draw color palette, sliders, etc
    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_currentHistogram);
    if (!h)
        return;

    // remove all previous palettes:
    removeColorPalettes();

    // create single palette:
    ColorPalette * palette = createColorPalette();
    _colorPalettes.append(palette);
//    _currentColorPalette = palette;

    _paletteHistogramMap.insert(palette, h);

    setupColorPalette(palette, h->outputStops, h->xmin, h->xmax, h->isDiscrete);
    zoomInterval(h->outputStops.first().first, h->outputStops.last().first);

    // add action in menu
    _menu.addAction(&_zoomFitSliders);
}

//*************************************************************************

/*!
    Method to draw histogram in rgb mode :
*/
void ColorManipulationView::drawRgbHistogram(int r, int g, int b)
{

    if (CheckIndex(r) || CheckIndex(g) || CheckIndex(b))
    {
        SD_TRACE("ColorManipulationView::drawRbgHistogram : index is out of bounds");
        return;
    }

    // Disable color palette menus
    _cmvSettings.interactiveColorPalette = false;

    // Create AllHistogramBands as HistogramItem
    HistogramView::drawRgbHistogram(r,g,b);
    _currentHistogram = 0;

    // remove all previous palettes:
    removeColorPalettes();

    // create 3 color palettes:
    int indices[] = {r,g,b};
    QTransform trs[] = {_cmvSettings.colorPaletteTransform2,
                        _cmvSettings.colorPaletteTransform2  * QTransform::fromTranslate(0.0, 0.075),
                        _cmvSettings.colorPaletteTransform2  * QTransform::fromTranslate(0.0, 0.15)};
    for (int i=0;i<3;i++)
    {
        CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_histograms[indices[i]]);
        if (!h)
            continue;
        ColorPalette * palette = createColorPalette(trs[i]);
        palette->setZValue(-i);
        _paletteHistogramMap.insert(palette, h);
        _colorPalettes.append(palette);
        setupColorPalette(palette, h->outputStops, h->xmin, h->xmax, h->isDiscrete);
    }

    zoomInterval(0.0, 1.0);

}

//*************************************************************************

/*!
    Slot to respond on slider position change
    double xpos is normalized slider position
*/
void ColorManipulationView::onSliderPositionChanged(Slider* slider, double xpos)
{

    ColorPalette * palette = qobject_cast<ColorPalette*>(sender());
    if (!palette)
    {
        SD_TRACE("ColorManipulationView::onSliderPositionChanged : palette = null");
        return;
    }

    CMVHistogramItem * h = getHistogramItem(palette);
    if (!h)
    {
        SD_TRACE("ColorManipulationView::onSliderPositionChanged : CMVHistogramItem is not found");
        return;
    }

    int index = palette->getSliderIndex(slider);
    if (index < 0 || index >= h->outputStops.size())
    {
        SD_TRACE("ColorManipulationView::onSliderPositionChanged : slider index is out of bounds");
        return;
    }

    h->outputStops[index].first = xpos;

    int hIndex = _histograms.indexOf(h);
    emit stopsChanged(hIndex, h->outputStops);

    // printStops(h->outputStops);

}

//*************************************************************************

/*!
    Slot to handle slider's color change
*/
void ColorManipulationView::onSliderColorChanged(Slider *slider, const QColor &c)
{
    ColorPalette * palette = qobject_cast<ColorPalette*>(sender());
    if (!palette)
    {
        SD_TRACE("ColorManipulationView::onSliderColorChanged : palette = null");
        return;
    }

    CMVHistogramItem * h = getHistogramItem(palette);
    if (!h)
    {
        SD_TRACE("ColorManipulationView::onSliderColorChanged : _currentHistogram = null");
        return;
    }

    int index = palette->getSliderIndex(slider);
    if (index < 0 || index >= h->outputStops.size())
    {
        SD_TRACE("ColorManipulationView::onSliderColorChanged : slider index is out of bounds");
        return;
    }

    h->outputStops[index].second = c;
    int hIndex = _histograms.indexOf(h);
    emit stopsChanged(hIndex, h->outputStops);

    // printStops(h->outputStops);
}

//*************************************************************************

/*!
    Slot to handle add slider action
*/
void ColorManipulationView::onAddSlider(Slider* slider)
{
    /*QGraphicsLineItem * sliderLine = */createSliderLine(slider);

    ColorPalette * palette = qobject_cast<ColorPalette*>(sender());
    if (!palette)
    {
        SD_TRACE("ColorManipulationView::onAddSlider : palette = null");
        return;
    }
    updateAllStops(palette);
}

//*************************************************************************

/*!
    Slot to handle add slider action
*/
void ColorManipulationView::onRemoveSlider()
{
    ColorPalette * palette = qobject_cast<ColorPalette*>(sender());
    if (!palette)
    {
        SD_TRACE("ColorManipulationView::onRemoveSlider : palette = null");
        return;
    }
    updateAllStops(palette);
}

//*************************************************************************

void ColorManipulationView::updateAllStops(ColorPalette * colorPalette)
{
    CMVHistogramItem * h = getHistogramItem(colorPalette);
    if (!h)
    {
        SD_TRACE("ColorManipulationView::updateAllStops : CMVHistogramItem is not found");
        return;
    }
    h->outputStops=colorPalette->getPalette();
    int hIndex = _histograms.indexOf(h);
    emit stopsChanged(hIndex, h->outputStops);

    // printStops(h->outputStops);
}

//*************************************************************************

/*!
    Slot to handle slider revert action
*/
void ColorManipulationView::onRevertSlider()
{
    ColorPalette * palette = qobject_cast<ColorPalette*>(sender());
    if (!palette)
    {
        SD_TRACE("ColorManipulationView::onRevertSlider : palette = null");
        return;
    }
    updateAllStops(palette);
}

//*************************************************************************

/*!
    Slot to handle zoom 'fit sliders'
*/
void ColorManipulationView::onFitSliders()
{
    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_currentHistogram);
    if (!h)
        return;
    zoomInterval(h->outputStops.first().first, h->outputStops.last().first);
}

//*************************************************************************

ColorManipulationView::CMVHistogramItem *ColorManipulationView::getHistogramItem(ColorPalette *colorPalette)
{
    return _paletteHistogramMap.value(colorPalette, 0);
}

//*************************************************************************

/*!
    Slot to handle discrete colors option of the histogram
*/
void ColorManipulationView::setDiscreteColors(bool value)
{
    foreach (ColorPalette * palette, _colorPalettes)
    {
        palette->setIsDiscrete(value);
        CMVHistogramItem * h = getHistogramItem(palette);
        if (!h)
            return;
        h->isDiscrete = palette->isDiscrete();
    }
}

//*************************************************************************

/*!
    Method to create color palette graphics item
*/
ColorPalette * ColorManipulationView::createColorPalette(const QTransform & transform)
{
    // draw color palette box
    ColorPalette * colorPalette = new ColorPalette();
    _histogramScene.addItem(colorPalette);
    colorPalette->setEditableSettings(_cmvSettings.interactiveColorPalette);

    if (transform.isIdentity())
    {
        colorPalette->setTransform(_cmvSettings.colorPaletteTransform);
    }
    else
    {
        colorPalette->setTransform(transform);
    }

    connect(colorPalette, SIGNAL(sliderPositionChanged(Slider*,double)), this, SLOT(onSliderPositionChanged(Slider*,double)));
    connect(colorPalette, SIGNAL(sliderColorChanged(Slider*,const QColor&)), this, SLOT(onSliderColorChanged(Slider*,QColor)));
    connect(colorPalette, SIGNAL(sliderAdded(Slider*)), this, SLOT(onAddSlider(Slider*)));
    connect(colorPalette, SIGNAL(sliderRemoved()), this, SLOT(onRemoveSlider()));


    return colorPalette;
}

//*************************************************************************

/*!
    Method to draw a color palette with n sliders from histogram info
    The map between line and slider is done with index in the _sliderLines and the index in the ColorPalette
*/
void ColorManipulationView::setupColorPalette(ColorPalette* palette, const QGradientStops & houtputStops, double xmin, double xmax, bool isDiscrete)
{

    palette->setupPalette(houtputStops, xmin, xmax, isDiscrete);

    // create slider lines from stops:
    for (int i=0;i<houtputStops.size();i++)
    {
        Slider* slider = palette->getSlider(i);
        /*QGraphicsLineItem * sliderLine = */createSliderLine(slider);
    }
}

//*************************************************************************

void ColorManipulationView::removeColorPalettes()
{
    if (!_colorPalettes.isEmpty())
    {
        foreach(ColorPalette * p, _colorPalettes)
        {
            _histogramScene.removeItem(p);
            delete p;
        }
        _colorPalettes.clear();
    }
}

//*************************************************************************

/*!
    Method to create slider line from color palette slider item
*/
QGraphicsLineItem * ColorManipulationView::createSliderLine(Slider * slider)
{
    QGraphicsLineItem * sliderLine = createLine();
    sliderLine->setParentItem(slider);

    // reset all parent transforms + align vertically with slider
    sliderLine->setTransform(sliderLineTransform(sliderLine, slider));
    sliderLine->setTransform(_settings.histogramTransform, true);
    sliderLine->setZValue(1.0);
    return sliderLine;
}

//*************************************************************************

}
