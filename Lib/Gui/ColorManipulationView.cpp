
// Qt
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QComboBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QGraphicsSceneMouseEvent>

// Project
#include "Core/Global.h"
#include "ColorManipulationView.h"


namespace Gui
{

//*************************************************************************

QGraphicsLineItem * createSliderLine()
{
    QGraphicsLineItem * line = new QGraphicsLineItem(
                0.0,0.0,
                0.0,1.0
                );
    line->setPen(QPen(QBrush(Qt::white), 0.0, Qt::DashLine));
    line->setZValue(2.0);
    return line;
}

inline double normalized(double x, double xmin, double xmax)
{
    return (x - xmin)/(xmax - xmin);
}

inline double unnormalized(double x, double xmin, double xmax)
{
    return x*(xmax - xmin) + xmin;
}

inline void updateToPartialMode(QGraphicsItem * hgi, double oldMin, double oldMax, double newMin, double newMax)
{
    hgi->setTransform(
                QTransform::fromTranslate(-normalized(newMin, oldMin, oldMax),0) *
                QTransform::fromScale((oldMax - oldMin)/(newMax - newMin),1)
                );
}

inline void copyPositions(const QGradientStops & src, QGradientStops & dst)
{
    if (src.size() != dst.size()) return;
    for(int i=0;i<src.size();i++)
    {
        dst[i].first = src[i].first;
    }
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
    _colorPalette(0),
    _removeSlider(tr("Remove slider"), this),
    _addSlider(tr("Add slider"), this),
    _revertSlider(tr("Center color"), this),
    _indexOfActionedSlider(-1)
{

    //    // setup timer:
    //    _updateDelayTimer.setSingleShot(true);
    //    connect(&_updateDelayTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerTimeout()));


    // setup context menu:
    setupViewContextMenu();

    // setup color picker
    _colorPicker.setWindowFlags(Qt::Popup);
    connect(&_colorPicker, SIGNAL(colorPicked(QColor)), this, SLOT(onColorPicked(QColor)));

    //    // setup value editor
    _valueEditor.setWindowFlags(Qt::Popup);
    _valueEditor.installEventFilter(this);
    _valueEditor.setAlignment(Qt::AlignRight);
    connect(&_valueEditor, SIGNAL(returnPressed()), this, SLOT(onValueEdited()));

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
    _colorPalette = 0;
    _sliderLines.clear();
    HistogramView::clear();
    createColorPalette();
}

//*************************************************************************

///*!
//    Method to reset sliders to default values
//*/
//void ColorManipulationView::resetToDefault()
//{
//    if (!_currentHistogram)
//        return;

//    if (_colorPalette)
//    {
//        // remove color palette with sliders
//        _histogramScene.removeItem(_colorPalette);
//    }
//    // redraw color palette & sliders & sliderlines
//    createColorPalette();

//    _ui->_transferFunction->setCurrentIndex(0);
//    _currentHistogram->transferFunctionName=_ui->_transferFunction->currentIndex();
//    _ui->_discreteColors->setChecked(false);
//    _currentHistogram->isDiscrete=_ui->_discreteColors->isChecked();

//    // reset current histogram outputstops
//    if (_currentHistogram->bandId > -1)
//    {
//        int index = _ui->_histList->currentIndex();
//        double a=(_currentHistogram->xmax2 - _currentHistogram->xmin2);
//        double b=_currentHistogram->xmin2;
//        if (a == 0)
//            a = 1;
//        if (_mode == RGB)
//            _currentHistogram->outputStops = Core::resetStops(index,a,b);
//        else
//            _currentHistogram->outputStops = Core::resetStops(-1,a,b);
//        // redraw histogram and sliders:
//        drawHistogram(index);
//    }
//    else
//    {
//        double a=(_currentHistogram->xmax2 - _currentHistogram->xmin2);
//        double b=_currentHistogram->xmin2;
//        _currentHistogram->outputStops = Core::resetStops(4,a,b);
//        // redraw
//        drawAllHistograms();
//    }


//    // notify
//    emit renderConfigurationChanged(_conf);

//}

////*************************************************************************

//void ColorManipulationView::initializeAllBandsHistogram()
//{
//    _allBandsHistogram.bandId=-1;
//    double xmin(1e10), xmax(-1e10), xmin2(xmin), xmax2(xmax);
//    foreach (HistogramDataView h, _histograms)
//    {
//        if (h.xmin < xmin) xmin = h.xmin;
//        if (h.xmax > xmax) xmax = h.xmax;
//        if (h.xmin2 < xmin2) xmin2 = h.xmin2;
//        if (h.xmax2 > xmax2) xmax2 = h.xmax2;

//    }
//    _allBandsHistogram.xmin=xmin;
//    _allBandsHistogram.xmin2=xmin2;
//    _allBandsHistogram.xmax=xmax;
//    _allBandsHistogram.xmax2=xmax2;
//    _allBandsHistogram.vxmin = _allBandsHistogram.xmin;
//    _allBandsHistogram.vxmax = _allBandsHistogram.xmax;
//    _allBandsHistogram.isDiscrete=false;
//    _allBandsHistogram.outputStops=Core::resetStops(4,xmax2-xmin2, xmin2);
//    _allBandsHistogram.transferFunctionName=_ui->_transferFunction->itemText(0);
//    _allBandsHistogram.graphicsItem=new QGraphicsItemGroup();
//    _allBandsHistogram.graphicsItem->setVisible(true);
//    _histogramScene.addItem(_allBandsHistogram.graphicsItem);

//}

//*************************************************************************

/*!
    Method to setup context menu
*/
void ColorManipulationView::setupViewContextMenu()
{

    _removeSlider.setVisible(false);
    _addSlider.setVisible(false);
    _revertSlider.setVisible(false);

    this->addAction(&_removeSlider);
    this->addAction(&_addSlider);
    this->addAction(&_revertSlider);

    // connect actions:
    connect(&_removeSlider, SIGNAL(triggered()), this, SLOT(onRemoveSlider()));
    connect(&_addSlider, SIGNAL(triggered()), this, SLOT(onAddSlider()));
    connect(&_revertSlider, SIGNAL(triggered()), this, SLOT(onRevertSlider()));

}

////*************************************************************************
///*!
// * \brief ColorManipulationView::setup
// * \param renderer
// * \param layer
// */
//void ColorManipulationView::setup(const Core::ImageRendererConfiguration &conf, const Core::ImageDataProvider * provider)
//{
//    const Core::HistogramRendererConfiguration * test = static_cast<const Core::HistogramRendererConfiguration*>(&conf);
//    if (!test)
//    {
//        SD_TRACE("ColorManipulationView::setup : Failed to cast ImageRendererConfiguration into HistogramRendererConfiguration");
//        return;
//    }
//    _conf = *test;
//    _initialConf = _conf;

//    int nbBands = _conf.normHistStops.size();

//    // configure toRGBMapping frame:
//    if (_conf.mode == Core::HistogramRendererConfiguration::RGB)
//    {
//        int nbBands = _conf.normHistStops.size();
//        _ui->_toRGBMapping->setVisible(true);
//        configureAChannel(_ui->_redChannel, _conf.toRGBMapping[0]+1, 1, nbBands);
//        configureAChannel(_ui->_greenChannel, _conf.toRGBMapping[1]+1, 1, nbBands);
//        configureAChannel(_ui->_blueChannel, _conf.toRGBMapping[2]+1, 1, nbBands);
//        configureAChannel(_ui->_grayChannel, _conf.toRGBMapping[0]+1, 1, nbBands);
//        _mode = RGB;
//    }
//    else
//    {
//        _ui->_toRGBMapping->setVisible(false);
//        _mode = GRAY;
//    }

//    // create histogram views
//    // 1) 1 band layer -> mode=GRAY, 1 histogram
//    // 2) Complex M bands layer -> mode=GRAY, 1 of 4*M histograms
//    // 3) Non-complex N bands layer -> { mode=RGB, 3 histograms | mode=GRAY, 1 of N histograms }

//    nbBands = qMin(3, nbBands);
//    for (int i=0; i<nbBands;i++)
//    {
//        int index = conf.toRGBMapping[i];
//        addHistogram(index, provider->getBandNames()[index],
//                     provider->getBandHistograms()[index]);
//    }

//    drawHistogram();
//}

////*************************************************************************
///*!
// * \brief ColorManipulationView::applyNewRendererConfiguration
// */
////void ColorManipulationView::applyNewRendererConfiguration()
////{
////    Core::HistogramImageRenderer* hRenderer = qobject_cast<Core::HistogramImageRenderer*>(_renderer);
////    if (!hRenderer)
////    {
////        SD_TRACE("ColorManipulationView::setup : Failed to cast renderer into HistogramLayerRenderer");
////        return;
////    }

////    // RGB mapping configuration
////    if (_mode == RGB)
////    {
////        Core::ImageRendererConfiguration conf = hRenderer->getConfiguration();
////        conf.toRGBMapping[0] = _ui->_redChannel->value()-1;
////        conf.toRGBMapping[1] = _ui->_greenChannel->value()-1;
////        conf.toRGBMapping[2] = _ui->_blueChannel->value()-1;
////        hRenderer->setConfiguration(conf);
////    }

//    // Histogram configuration
////    int index = _currentHistogram->bandId;
////    Core::HistogramRendererConfiguration histConf = hRenderer->getHistConfiguration();
////    if (index > -1)
////    {
////        histConf.isDiscreteValues[index] = _currentHistogram->isDiscrete;
////        histConf.normHistStops[index] = computeStopsFromValues(_currentHistogram->outputStops, _currentHistogram->xmin, _currentHistogram->xmax);
////        histConf.transferFunctions[index] = Core::HistogramRendererConfiguration::getTransferFunctionByName(_currentHistogram->transferFunctionName);
////    }
////    else
////    { // all bands -> update all histograms and histConf
////        for (int i=0;i<_histograms.size();i++)
////        {
////            copyPositions(_currentHistogram->outputStops,_histograms[i].outputStops);
////            histConf.isDiscreteValues[i]=_histograms[i].isDiscrete;
////            histConf.normHistStops[i]=computeStopsFromValues(_histograms[i].outputStops, _histograms[i].xmin, _histograms[i].xmax);
////            histConf.transferFunctions[i]=Core::HistogramRendererConfiguration::getTransferFunctionByName(_histograms[i].transferFunctionName);
////        }
////    }
////    hRenderer->setHistConfiguration(histConf);
////}


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


////*************************************************************************

///*!
//    Method to add histogram data. For single band display, add only one histogram. For RGB display, add exactly 3 histograms.
//*/
//void ColorManipulationView::addHistogram(int id, const QString &name, const QVector<double> & data)
//{
//    _ui->_histList->addItem(name, id);

//    int index = _histograms.size();
//    _histograms.resize(index+1);
//    _histograms[index].bandId=id;
//    _histograms[index].xmin=_conf.minValues[id];
//    _histograms[index].xmax=_conf.maxValues[id];
//    _histograms[index].xmin2=_conf.qMinValues[id];
//    _histograms[index].xmax2=_conf.qMaxValues[id];
//    _histograms[index].transferFunctionName=_conf.transferFunctions[id]->getName();
//    _histograms[index].isDiscrete=_conf.isDiscreteValues[id];

//    if (_conf.mode == Core::HistogramRendererConfiguration::GRAY)
//    {
//        _histograms[index].outputStops=computeValuesFromStops(_conf.normHistStops[id], _histograms[index].xmin, _histograms[index].xmax);
//    }
//    else if (_conf.mode == Core::HistogramRendererConfiguration::RGB)
//    {
//        _histograms[index].outputStops=computeValuesFromStops(_conf.normRGBHistStops[id], _histograms[index].xmin, _histograms[index].xmax);
//    }

//    // draw histogram bars :
//    _histograms[index].graphicsItem = createHistogramGraphicsItem(data, _settings.dataPen);
//    _histograms[index].graphicsItem->setVisible(false);
//    _histogramScene.addItem(_histograms[index].graphicsItem);

//}

//*************************************************************************
/*!
    Method to draw histogram in gray or rgb mode :
*/
void ColorManipulationView::drawSingleHistogram(int index)
{

    if (index < 0 || index >= _histograms.size())
    {
        SD_TRACE("ColorManipulationView::drawSingleHistogram : index is out of bounds");
        return;
    }

    HistogramView::drawSingleHistogram(index);

    // draw color palette, sliders, etc
    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_currentHistogram);
    if (!h)
        return;

    drawColorPalette(h->outputStops, h->xmin, h->xmax, h->isDiscrete);
    zoomInterval(h->outputStops.first().first, h->outputStops.last().first);
}

////*************************************************************************

///*!
//    Method to draw histogram in gray or rgb mode :
//*/
//void ColorManipulationView::drawHistogram()
//{

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

//}

////*************************************************************************

///*!
//    Method to set transfer function names
//*/
//void ColorManipulationView::setTransferFunctionNames(const QStringList &transferFunctionNames)
//{
//    _ui->_transferFunction->clear();
//    foreach (QString tf, transferFunctionNames)
//    {
//        _ui->_transferFunction->addItem(tf);
//    }

//    // set visible if specified at least one transfer function
//    if (!_ui->_transferFunction->isVisible())
//    {
//        _ui->_transferFunction->setVisible(true);
//        _ui->_transferFunctionLabel->setVisible(true);
//    }
//}

////*************************************************************************

///*!
//    Slot called when update time is timeout and one need send updated color schema
//*/
//void ColorManipulationView::onUpdateTimerTimeout()
//{
//    emit renderConfigurationChanged(_conf);
//}

//*************************************************************************

/*!
    Method to update histogram data: stops and outputstops
*/
void ColorManipulationView::updateHistogramData(int index, double xpos, const QColor &c, bool notificationDelayed)
{

    if (!_currentHistogram)
        return;

    CMVHistogramItem * h = static_cast<CMVHistogramItem*>(_currentHistogram);

    if (!h)
        return;

    if (index < 0)
    {
        h->outputStops=_colorPalette->getPalette();
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
        emit configurationChanged(h->outputStops);
    }
}

//*************************************************************************

/*!
    Slot to respond on slider position changed
*/
void ColorManipulationView::onSliderPositionChanged(int index, double position)
{
    if (index < 0 || index > _sliderLines.size() - 1 || !_currentHistogram)
        return;

    QGraphicsLineItem * sliderLine = _sliderLines[index];
    sliderLine->setTransform(_settings.histogramTransform);
//    double npos=normalized(position, _currentHistogram->xmin, _currentHistogram->xmax);
    double npos=position;
    sliderLine->setTransform(QTransform::fromTranslate(npos, 0), true);

    // update histogram data
    updateHistogramData(index, position, QColor(), true);

}

//*************************************************************************

/*!
    Slot to handle add slider action
*/
void ColorManipulationView::onAddSlider()
{
    QPointF position = _addSlider.data().toPointF();
    if (position.isNull())
        return;

    int index=-1;

    if (_colorPalette->addSlider(position, &index))
    {
        QGraphicsLineItem * sliderLine = createSliderLine();
        _histogramScene.addItem(sliderLine);
        sliderLine->setTransform(_settings.histogramTransform);
        sliderLine->setTransform(QTransform::fromTranslate(position.x(), 0), true);
        sliderLine->setZValue(1.0);
        _sliderLines.insert(index, sliderLine);
    }

    // update histogram data
    updateHistogramData();

}

//*************************************************************************

/*!
    Slot to handle add slider action
*/
void ColorManipulationView::onRemoveSlider()
{
    int index = _indexOfActionedSlider;
    _indexOfActionedSlider=-1;
    if (index < 0 || index > _sliderLines.size() - 1)
        return;

    if (_colorPalette->removeSliderAtIndex(index))
    {
        QGraphicsLineItem * sliderLine = _sliderLines[index];
        _sliderLines.removeAt(index);
        _histogramScene.removeItem(sliderLine);
    }

    // update histogram data
    updateHistogramData();

}

//*************************************************************************

/*!
    Slot to handle color pick action
*/
void ColorManipulationView::onColorPicked(QColor c)
{
    int index = _indexOfActionedSlider;
    _indexOfActionedSlider=-1;
    if (index < 0 || index > _sliderLines.size() - 1)
        return;

    _colorPalette->setColorOfSliderAtIndex(index, c);
    _colorPicker.hide();

    // update histogram data
    updateHistogramData(index, -12345, c);

}

//*************************************************************************

/*!
    Slot to handle value modification by user
*/
void ColorManipulationView::onValueEdited()
{
    bool ok=false;
    double newvalue = _valueEditor.text().toDouble(&ok);
    if (ok)
    {
        int index = _indexOfActionedSlider;
        _indexOfActionedSlider=-1;
        if (index < 0 || index > _sliderLines.size() - 1
                || !_currentHistogram)
            return;
        newvalue = normalized(newvalue, _currentHistogram->xmin, _currentHistogram->xmax);
        _colorPalette->setSliderValueAtIndex(index, newvalue);
        _valueEditor.hide();
        _colorPalette->highlightSliderTextAtIndex(index, false);

    }
}

//*************************************************************************

/*!
    Slot to handle slider revert action
*/
void ColorManipulationView::onRevertSlider()
{
    int index = _indexOfActionedSlider;
    _indexOfActionedSlider=-1;
    if (index < 0 || index > _sliderLines.size() - 1)
        return;

    _colorPalette->resetColorOfSliderAtIndex(index);

    updateHistogramData();

}

////*************************************************************************

///*!
//    Slot to handle isComplete radio button clicked
//*/
//void ColorManipulationView::onDisplayCompleteHist(bool checked)
//{
//    if (!_currentHistogram || !checked)
//        return;

//    double newMin, newMax;
//    newMin = _currentHistogram->xmin;
//    newMax = _currentHistogram->xmax;

//    transformAllItems(newMin, newMax);
//}

////*************************************************************************

///*!
//    Slot to handle the action that displays the histogram fitted to slider positions
//*/
//void ColorManipulationView::onDisplayPartialHist(bool checked)
//{
//    if (!_currentHistogram || !checked)
//        return;

//    double newMin, newMax;
//    newMin = _currentHistogram->outputStops.first().first;
//    newMax = _currentHistogram->outputStops.last().first;

//    transformAllItems(newMin, newMax);
//}

////*************************************************************************

///*!
//    Slot to handle histogram list current index changed
//*/
//void ColorManipulationView::onHistListIndexChanged(int i)
//{
//    drawHistogram(i);
//}

////*************************************************************************

///*!
//    Slot to handle histogram list current index changed
//*/
//void ColorManipulationView::onTransferFunctionChanged(QString tf)
//{
//    if (!_currentHistogram || _currentHistogram->transferFunctionName == tf)
//        return;

//    _currentHistogram->transferFunctionName = tf;
//    emit renderConfigurationChanged(_conf);
//}

////*************************************************************************

///*!
//    Slot to handle discrete colors option of the histogram
//*/
//void ColorManipulationView::onDiscreteColorsClicked(bool checked)
//{
//    _colorPalette->setIsDiscrete(checked);
//    _currentHistogram->isDiscrete = _colorPalette->isDiscrete();
//    emit renderConfigurationChanged(_conf);
//}

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
void ColorManipulationView::drawColorPalette(const QGradientStops & houtputStops, double xmin, double xmax, bool isDiscrete)
{
    // remove previous slider lines :
    foreach (QGraphicsLineItem * line, _sliderLines)
    {
        _histogramScene.removeItem(line);
    }
    _sliderLines.clear();

    // draw default/saved slider lines:
    for (int i=0;i<houtputStops.size();i++)
    {
        QGraphicsLineItem * sliderLine = createSliderLine();
        _histogramScene.addItem(sliderLine);
        sliderLine->setTransform(_settings.histogramTransform);
//        double xpos=normalized(houtputStops[i].first, hvxmin, hvxmax);
        double xpos=houtputStops[i].first;
        sliderLine->setTransform(QTransform::fromTranslate(xpos, 0), true);
        sliderLine->setZValue(1.0);
        _sliderLines << sliderLine;
    }

    _colorPalette->setupPalette(houtputStops, xmin, xmax, isDiscrete);
    connect(_colorPalette, SIGNAL(sliderPositionChanged(int,double)), this, SLOT(onSliderPositionChanged(int,double)));
}

//*************************************************************************

/*!
    Method to create color palette graphics item
*/
void ColorManipulationView::createColorPalette()
{
    // draw color palette box
    _colorPalette = new ColorPalette;
    _histogramScene.addItem(_colorPalette);
    _colorPalette->setTransform(_cmvSettings.colorPaletteTransform);
}

//*************************************************************************

void ColorManipulationView::onContextMenuRequested(QPoint p)
{
    QPointF scenePt = _ui->_histogramView->mapToScene(p);
    _menu.clear();
    if (_histograms.size() > 0)
    {
        QGraphicsItem * itemUnderMouse = _histogramScene.itemAt(scenePt, QTransform());

        if (_colorPalette->itemIsSlider(itemUnderMouse) && _cmvSettings.interactiveColorPalette )
        { // if clicked on slider :
            _menu.addActions(this->actions());
            // open context menu : - remove slider if nb(sliders) > 3
            if (_colorPalette->getNbOfSliders() > 3)
            {
                _addSlider.setVisible(false);
                _removeSlider.setVisible(true);
                _revertSlider.setVisible(true);
                _indexOfActionedSlider = _colorPalette->getSliderIndex(itemUnderMouse);
            }
            else if (_colorPalette->getNbOfSliders() == 3)
            {
                _addSlider.setVisible(false);
                _removeSlider.setVisible(false);
                _revertSlider.setVisible(true);
                _indexOfActionedSlider = _colorPalette->getSliderIndex(itemUnderMouse);
            }
        }
        else if (_colorPalette->itemIsPalette(itemUnderMouse) && _cmvSettings.interactiveColorPalette)
        { // if clicked on color palette rectangle
            _menu.addActions(this->actions());
            _removeSlider.setVisible(false);
            _revertSlider.setVisible(false);
            _addSlider.setVisible(true);
            _addSlider.setData(itemUnderMouse->mapFromScene(scenePt));
        }
        else
        {
            _menu.addActions(_ui->_histogramView->actions());
            HistogramView::onContextMenuRequested(p);
            return;
        }
        _menu.popup(_ui->_histogramView->mapToGlobal(p));
    }

}

//*************************************************************************

/*!
*/
bool ColorManipulationView::eventFilter(QObject * object, QEvent * event)
{
    if (&_histogramScene == object && _histograms.size() > 0)
    {
        if (event->type() == QEvent::GraphicsSceneMouseDoubleClick)
        {
            QGraphicsSceneMouseEvent* e = static_cast<QGraphicsSceneMouseEvent*>(event);
            QGraphicsItem * itemUnderMouse = _histogramScene.itemAt(e->scenePos(), QTransform());
            if (_colorPalette->itemIsSlider(itemUnderMouse) && _cmvSettings.interactiveColorPalette)
            { // if double-clicked on slider :
                switch (e->button())
                {
                case Qt::LeftButton:
                { // open color picker
                    _colorPicker.move(e->screenPos());
                    _colorPicker.show();
                    QPoint p = QWidget::mapToGlobal(QPoint(0,0));
                    if (_colorPicker.x() + _colorPicker.width() > p.x() + this->width())
                    {
                        _colorPicker.move(e->screenPos() - QPoint(_colorPicker.width(),0));
                    }
                    _indexOfActionedSlider = _colorPalette->getSliderIndex(itemUnderMouse);
                    e->accept();
                    return true;
                }
                }
            }
            else if (_colorPalette->itemIsSliderText(itemUnderMouse))
            { // if double-clicked on slider-text :
                switch (e->button())
                {
                case Qt::LeftButton:
                { // open value editor
                    _indexOfActionedSlider=_colorPalette->getSliderIndex(itemUnderMouse->parentItem());
                    if (_indexOfActionedSlider < 0)
                        return false;
                    double value = _colorPalette->getValue(_indexOfActionedSlider);
                    if (value < -12344)
                        return false;

                    QPoint p = QWidget::mapToGlobal(QPoint(0,0));
                    int x = e->screenPos().x();
                    int y = p.y() + _ui->_histogramView->height();
                    _valueEditor.move(QPoint(x,y));
                    _valueEditor.setText(QString("%1").arg(value));
                    _valueEditor.show();
                    _valueEditor.resize(60,_valueEditor.height());
                    if (_valueEditor.x() + _valueEditor.width() > p.x() + this->width())
                    {
                        _valueEditor.move(QPoint(x - _valueEditor.width(),y));
                    }

                    // highlight slider text:
                    _colorPalette->highlightSliderTextAtIndex(_indexOfActionedSlider);

                    e->accept();
                    return true;
                }
                }
            }
        }
    }
    else if (&_valueEditor == object)
    {
        valueEditorEvents(event);
    }
    return QWidget::eventFilter(object, event);
}

//*************************************************************************

/*!
 * Handle value editor key and mouse events
*/
void ColorManipulationView::valueEditorEvents(QEvent * event)
{
    if (event->type() == QEvent::KeyPress)
    { // Hide valueEditor when user presses escape key
        QKeyEvent * ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape && _valueEditor.isVisible())
        {
            _valueEditor.hide();
            _colorPalette->highlightSliderTextAtIndex(_indexOfActionedSlider, false);
            _indexOfActionedSlider=-1;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    { // Hide valueEditor when user clicks somewhere else
        QMouseEvent * me = static_cast<QMouseEvent*>(event);
        if (!_valueEditor.rect().contains(me->pos()))
        {
            _valueEditor.hide();
            _colorPalette->highlightSliderTextAtIndex(_indexOfActionedSlider, false);
            _indexOfActionedSlider=-1;
        }
    }
}

//*************************************************************************

}
