
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

QGraphicsLineItem * createSliderLine(QGraphicsItem * parent=0)
{
    QGraphicsLineItem * line = new QGraphicsLineItem(
                0.0,0.0,
                0.0,1.0,
                parent);
    line->setPen(QPen(QBrush(Qt::white), 0.0, Qt::DashLine));
    line->setZValue(2.0);
    return line;
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
//    _colorPalette(0),
    _zoomFitSliders(tr("Fit sliders"), this),
    _currentColorPalette(0)
{

    // setup context menu:
    setupViewContextMenu();

    _histogramScene.installEventFilter(this);

    _settings.histogramTransform =
            QTransform::fromScale(1.0,_cmvSettings.histOverPaletteRatio);

    _cmvSettings.colorPaletteTransform =
            QTransform::fromScale(1.0, 1.0 -_cmvSettings.histOverPaletteRatio) *
            QTransform::fromTranslate(0.0, _histogramScene.height()*_cmvSettings.histOverPaletteRatio);


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
    _currentColorPalette = 0;
    _colorPalettes.clear();
    HistogramView::clear();
//    createColorPalette();
}

//*************************************************************************

/*!
    Method to setup context menu
*/
void ColorManipulationView::setupViewContextMenu()
{
    _menu.addAction(&_zoomFitSliders);
    // connect actions:
    connect(&_zoomFitSliders, SIGNAL(triggered()), this, SLOT(onFitSliders()));
}

//*************************************************************************

/*!
 * Method to add histogram data.
 * \param GradientStops are normalized color stops for the ColorPalette
 * \param QVector<double> data is normalized histogram values
*/
void ColorManipulationView::addHistogram(const QGradientStops & normStops, const QVector<double> & data, double xmin, double xmax)
{
    int index = _histograms.size();
    _histograms.append(new CMVHistogramItem());

    if (!setHistogram(index, normStops, data, xmin, xmax))
    {
        HistogramItem * h = _histograms.takeLast();
        delete h;
    }
}

//*************************************************************************

/*!
    Method to set histogram data at index.
*/
bool ColorManipulationView::setHistogram(int index, const QGradientStops & normStops, const QVector<double> & data, double xmin, double xmax)
{
    if (index < 0 || index >= _histograms.size())
    {
        return false;
    }
    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_histograms[index]);
    if (!h)
        return false;

    HistogramView::setHistogram(index, data, xmin, xmax);
    h->isDiscrete=false;
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
    if (!_colorPalettes.isEmpty())
    {
        foreach(ColorPalette * p, _colorPalettes)
        {
            _histogramScene.removeItem(p);
            delete p;
        }
        _colorPalettes.clear();
    }

    // create single palette:
    ColorPalette * palette = createColorPalette();
    _colorPalettes.append(palette);
    _currentColorPalette = palette;

    setupColorPalette(palette, h->outputStops, h->xmin, h->xmax, h->isDiscrete);
    zoomInterval(h->outputStops.first().first, h->outputStops.last().first);
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

//    // Disable color palette menus
//    _cmvSettings.interactiveColorPalette = false;

//    // create AllHistogramBands as CMVHistogramItem
//    if (!_allBandsHistogram)
//    {
//        CMVHistogramItem * allBands = new CMVHistogramItem();
//        int indices[] = {r,g,b};
//        for (int i=0;i<_histograms.size();i++)
//        {
//            CMVHistogramItem* hItem = static_cast<CMVHistogramItem*>(_histograms[indices[i]]);
//            if (!hItem)
//                return;
//            allBands->outputStops << hItem->outputStops;
//        }
//        orderStops(&allBands->outputStops);
//        _allBandsHistogram = allBands;
//    }

//    HistogramView::drawRgbHistogram(r,g,b);

//    // draw color palette, sliders, etc
//    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_currentHistogram);
//    if (!h)
//        return;

//    drawColorPalette(h->outputStops, h->xmin, h->xmax, h->isDiscrete);

//    // Setup groups :
//    QMap<int, int> map;
//    map.insert(0, 0); map.insert(1, 0); map.insert(2, 0);
//    map.insert(3, 1); map.insert(4, 1); map.insert(5, 1);
//    _colorPalette->setupSliderGroups(map, 2);

//    zoomInterval(h->outputStops.first().first, h->outputStops.last().first);


//    // Check if there is only one histogram added in the list:
//    if (_histograms.size() == 1)
//    {
//        _mode = GRAY;
//        _ui->_isAllBands->setEnabled(false);
//    }
//    else if (_histograms.size() == 3)
//    {
//        _mode = RGB;
//        _ui->_isAllBands->setEnabled(true);
//        initializeAllBandsHistogram();
//    }
//    else
//    {
//        SD_TRACE("ColorManipulationView::drawHistogram() : user should add 1 (gray mode) or 3 (rgb mode) histogram datas");
//        return;
//    }

//    if (_ui->_transferFunction->count()==0)
//    {
//        // hide transfer function option
//        _ui->_transferFunction->setVisible(false);
//        _ui->_transferFunctionLabel->setVisible(false);
//    }

//    _ui->_histList->setEnabled(true);
//    _ui->_histList->setCurrentIndex(0);
//    _ui->_isPartial->setEnabled(true);
//    _ui->_isPartial->setChecked(true);
//    _ui->_isComplete->setEnabled(true);

//    _ui->_discreteColors->setEnabled(true);
//    _ui->_revert->setEnabled(true);

//    _ui->_transferFunction->setEnabled(true);
//    _ui->_transferFunction->setCurrentIndex(0);

//    drawHistogram(0);

}

//*************************************************************************

/*!
    Method to update histogram data: stops and outputstops
*/
void ColorManipulationView::updateHistogramItem(int index, double xpos, const QColor &c, bool notificationDelayed)
{

    if (!_currentHistogram)
        return;

    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_currentHistogram);
    if (!h)
        return;

    if (index < 0)
    {
//        h->outputStops=_colorPalette->getPalette();
    }
    else if (index >= 0 && xpos > -12344)
    {
        h->outputStops[index].first = xpos;
    }
    else if (index >= 0 && c.isValid())
    {
        h->outputStops[index].second = c;
    }

    // notify about change:
    if (!notificationDelayed)
    {
        int hIndex = _histograms.indexOf(h);
        emit stopsChanged(hIndex, h->outputStops);
    }
}

//*************************************************************************

/*!
    Slot to respond on slider position change
*/
void ColorManipulationView::onSliderPositionChanged(int index, double position)
{
//    if (index < 0 || index > _sliderLines.size() - 1 || !_currentHistogram)
//        return;

//    QGraphicsLineItem * sliderLine = _sliderLines[index];
//    sliderLine->setTransform(_settings.histogramTransform);
////    double npos=normalized(position, _currentHistogram->xmin, _currentHistogram->xmax);
//    double npos=position;
//    sliderLine->setTransform(QTransform::fromTranslate(npos, 0), true);

    // update histogram data
    updateHistogramItem(index, position, QColor(), true);

}

//*************************************************************************

/*!
    Slot to respond on slider position change
*/
void ColorManipulationView::onSliderMouseRelease(int index, double position)
{
//    if (index < 0 || index > _sliderLines.size() - 1 || !_currentHistogram)
//        return;

    // update histogram data
    updateHistogramItem(index, position);

}

//*************************************************************************

/*!
    Slot to handle add slider action
*/
void ColorManipulationView::onAddSlider()
{
//    QPointF position = _addSlider.data().toPointF();
//    if (position.isNull())
//        return;

    int index=-1;

//    if (_colorPalette->addSlider(position, &index))
//    {
//        QGraphicsLineItem * sliderLine = createSliderLine();
//        _histogramScene.addItem(sliderLine);
//        sliderLine->setTransform(_settings.histogramTransform);
//        sliderLine->setTransform(QTransform::fromTranslate(position.x(), 0), true);
//        sliderLine->setZValue(1.0);
////        _sliderLines.insert(index, sliderLine);
//    }

    // update histogram data
    updateHistogramItem();

}

//*************************************************************************

/*!
    Slot to handle add slider action
*/
void ColorManipulationView::onRemoveSlider()
{
//    int index = _indexOfActionedSlider;
//    _indexOfActionedSlider=-1;
//    if (index < 0 || index > _sliderLines.size() - 1)
//        return;

//    if (_colorPalette->removeSliderAtIndex(index))
//    {
//        QGraphicsLineItem * sliderLine = _sliderLines[index];
//        _sliderLines.removeAt(index);
//        _histogramScene.removeItem(sliderLine);
//    }

    // update histogram data
    updateHistogramItem();

}

////*************************************************************************

///*!
//    Slot to handle color pick action
//*/
//void ColorManipulationView::onColorPicked(QColor c)
//{
//    int index = _indexOfActionedSlider;
//    _indexOfActionedSlider=-1;
////    if (index < 0 || index > _sliderLines.size() - 1)
////        return;

////    _colorPalette->setColorOfSliderAtIndex(index, c);
////    _colorPicker.hide();

//    // update histogram data
//    updateHistogramItem(index, -12345, c);

//}

////*************************************************************************

///*!
//    Slot to handle value modification by user
//*/
//void ColorManipulationView::onValueEdited()
//{

//    bool ok=false;
//    double newvalue = _valueEditor.text().toDouble(&ok);
//    if (ok)
//    {
//        int index = _indexOfActionedSlider;
//        _indexOfActionedSlider=-1;
////        if (index < 0 || index > _sliderLines.size() - 1
////                || !_currentHistogram)
////            return;
////        newvalue = normalized(newvalue, _currentHistogram->xmin, _currentHistogram->xmax);
////        _colorPalette->setSliderValueAtIndex(index, newvalue);
////        _valueEditor.hide();
////        _colorPalette->highlightSliderTextAtIndex(index, false);

//        // notify about the change. Slider position change slot does not notify
//        CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_currentHistogram);
//        if (h)
//        {
//            int hIndex = _histograms.indexOf(h);
//            emit stopsChanged(hIndex, h->outputStops);
//        }
//    }
//}

//*************************************************************************

/*!
    Slot to handle slider revert action
*/
void ColorManipulationView::onRevertSlider()
{
//    int index = _indexOfActionedSlider;
//    _indexOfActionedSlider=-1;
//    if (index < 0 || index > _sliderLines.size() - 1)
//        return;

//    _colorPalette->resetColorOfSliderAtIndex(index);

    updateHistogramItem();

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

/*!
    Slot to handle discrete colors option of the histogram
*/
void ColorManipulationView::onDiscreteColorsClicked(bool checked)
{
//    _colorPalette->setIsDiscrete(checked);
    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_currentHistogram);
    if (!h)
        return;
//    h->isDiscrete = _colorPalette->isDiscrete();
}

////*************************************************************************

///*!
//    Slot to handle when all bands option is checked/unchecked
//*/
//void ColorManipulationView::onIsAllBandsClicked(bool checked)
//{
//    if (checked)
//    {
//        _ui->_histList->setEnabled(false);
//        _ui->_discreteColors->setEnabled(false);
//        _ui->_transferFunction->setEnabled(false);
//        drawAllHistograms();
//    }
//    else
//    {
//        _ui->_histList->setEnabled(true);
//        _ui->_discreteColors->setEnabled(true);
//        _ui->_transferFunction->setEnabled(true);

//        // set invisible all histograms:
//        for (int i=0;i<_histograms.size();i++)
//        {
//            _histograms[i].graphicsItem->setVisible(false);
//            _histograms[i].xmin = _allBandsHistogram.xmin;
//            _histograms[i].xmax = _allBandsHistogram.xmax;
//            _allBandsHistogram.graphicsItem->removeFromGroup(_histograms[i].graphicsItem);
//        }
//        // draw
//        drawHistogram(_ui->_histList->currentIndex());
//    }

//}

////*************************************************************************

///*!
//    on Show event
//*/
//void ColorManipulationView::showEvent(QShowEvent * event)
//{
//    if (event->type() == QEvent::Show)
//    {
//        _ui->_ColorManipulationView->fitInView(_histogramScene.sceneRect());
//    }
//}

////*************************************************************************

///*!
//    on Resize event
//*/
//void ColorManipulationView::resizeEvent(QResizeEvent * event)
//{
//    if (event->type() == QEvent::Resize)
//    {
//        _ui->_ColorManipulationView->fitInView(_histogramScene.sceneRect());
//    }
//}

////*************************************************************************

///*!
//    Method to transform all graphics items
//*/
//void ColorManipulationView::transformAllItems(double newMin, double newMax)
//{
//    // transform histogram graphics item
//    QTransform tr = _currentHistogram->graphicsItem->transform();
//    updateToPartialMode(_currentHistogram->graphicsItem, _currentHistogram->vxmin, _currentHistogram->vxmax, newMin, newMax);
//    _currentHistogram->graphicsItem->setTransform(tr, true);

//    // transform slider lines
//    foreach (QGraphicsLineItem * line, _sliderLines)
//    {
//        QTransform tr = line->transform();
//        updateToPartialMode(line, _currentHistogram->vxmin, _currentHistogram->vxmax, newMin, newMax);
//        line->setTransform(tr, true);
//    }

//    // transform slider inner positions
//    _colorPalette->setMinMaxRanges(newMin, newMax);

//    // update visual histogram range
//    _currentHistogram->vxmin=newMin;
//    _currentHistogram->vxmax=newMax;
//}

////*************************************************************************

///*!
//    Method to draw the histogram at index
//*/
//void ColorManipulationView::drawHistogram(int index)
//{

//    if (index < 0 || index >=_histograms.size())
//        return;

//    // make invisible current histogram:
//    if (_currentHistogram && _currentHistogram->graphicsItem)
//        _currentHistogram->graphicsItem->setVisible(false);

////    HistogramDataView & h = _histograms[index];
//    _currentHistogram = &_histograms[index];

//    drawHistogramGraphicsItem(_currentHistogram, _settings.dataPen);

//    // setup transfer function and isDiscrete coulours :
//    _ui->_discreteColors->setChecked(_currentHistogram->isDiscrete);
//    int i = _ui->_transferFunction->findText(_currentHistogram->transferFunctionName);
//    if (i < 0)
//    {
//        i = 0;
//        _currentHistogram->transferFunctionName=_ui->_transferFunction->itemText(i);
//    }
//    _ui->_transferFunction->setCurrentIndex(i);

//    // draw color palette, sliders, etc in 100% view mode
//    drawColorPalette(_currentHistogram->outputStops, _currentHistogram->vxmin, _currentHistogram->vxmax, _currentHistogram->isDiscrete);

//    // default choice is 95% of histogram to display
//    _ui->_isPartial->setChecked(true);
//    onDisplayPartialHist(true);

//}

////*************************************************************************

///*!
//    Method to draw all histograms
//*/
//void ColorManipulationView::drawAllHistograms()
//{
//    _currentHistogram = &_allBandsHistogram;
//    // draw all histograms:
//    QList<QColor> dataColors = QList<QColor>()
//            << QColor(255,0,0,81)
//            << QColor(0,255,0,81)
//            << QColor(0,0,255,81);
//    for (int i=0;i<_histograms.size();i++)
//    {
//        HistogramDataView & h = _histograms[i];
//        drawHistogramGraphicsItem(&h, QPen(dataColors[i], 0.0));

//        // set stops of the band histogram equal to the stops of allBands;
//        copyPositions(_currentHistogram->outputStops,h.outputStops);
//        _allBandsHistogram.graphicsItem->addToGroup(h.graphicsItem);
//    }

//    // draw color palette, sliders, etc in 100% view mode
//    drawColorPalette(_currentHistogram->outputStops, _currentHistogram->vxmin, _currentHistogram->vxmax, false);

//    // default choice is 95% of histogram to display
//    _ui->_isPartial->setChecked(true);
//    onDisplayPartialHist(true);


//}

//*************************************************************************

/*!
    Method to draw a color palette with n sliders from histogram info
    The map between line and slider is done with index in the _sliderLines and the index in the ColorPalette
*/
void ColorManipulationView::setupColorPalette(ColorPalette* palette, const QGradientStops & houtputStops, double xmin, double xmax, bool isDiscrete)
{


//    // remove previous slider lines :
//    foreach (QGraphicsLineItem * line, _sliderLines)
//    {
//        _histogramScene.removeItem(line);
//        delete line;
//    }
//    _sliderLines.clear();

    palette->setupPalette(houtputStops, xmin, xmax, isDiscrete);

    // create slider lines from stops:
    for (int i=0;i<houtputStops.size();i++)
    {
        QGraphicsLineItem * sliderLine = createSliderLine();
        _histogramScene.addItem(sliderLine);
        sliderLine->setTransform(_settings.histogramTransform);
        sliderLine->setParentItem(palette->getSlider(i));

        double xpos=houtputStops[i].first;
        sliderLine->setTransform(QTransform::fromTranslate(xpos, 0), true);
        sliderLine->setZValue(1.0);
//        _sliderLines << sliderLine;
    }


//    connect(palette, SIGNAL(sliderPositionChanged(int,double)), this, SLOT(onSliderPositionChanged(int,double)));
//    connect(palette, SIGNAL(sliderMouseRelease(int,double)), this, SLOT(onSliderMouseRelease(int,double)));

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
    if (transform.isIdentity())
    {
        colorPalette->setTransform(_cmvSettings.colorPaletteTransform);
    }
    return colorPalette;
}

//*************************************************************************

/*!
*/
bool ColorManipulationView::eventFilter(QObject * object, QEvent * event)
{
    return QWidget::eventFilter(object, event);
}

//*************************************************************************

}
