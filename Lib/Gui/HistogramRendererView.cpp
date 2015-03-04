// STD
#include <vector>

// Qt
#include <qmath.h>
#include <QGraphicsLineItem>
#include <QLinearGradient>
#include <QGraphicsSceneMoveEvent>
#include <QHBoxLayout>

// Project
#include "HistogramRendererView.h"
#include "Core/Global.h"
#include "Core/ImageRenderer.h"
#include "Core/ImageDataProvider.h"

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

QGraphicsItemGroup * createAxesGroup(const QPen & pen)
{
    QGraphicsItemGroup * axes = new QGraphicsItemGroup();
    QGraphicsLineItem * axisX = new QGraphicsLineItem(
                0.0, 1.0, 1.0, 1.0
                );
    axisX->setPen(pen);
    axes->addToGroup(axisX);

    QGraphicsLineItem * axisY = new QGraphicsLineItem(
                0.0, 0.0, 0.0, 1.0
                );
    axisY->setPen(pen);
    axes->addToGroup(axisY);
    return axes;
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

void configureAChannel(QSpinBox * spinbox, int currentValue, int minValue, int maxValue)
{
    spinbox->setMinimum(minValue);
    spinbox->setMaximum(maxValue);
    spinbox->setValue(currentValue);
}

//*************************************************************************

/*!

        \class HistogramRendererView
        \ingroup Gui
        \brief This class implements the histogram color manipulation widget. This is
        used to enhance color contrast and visualization using image histogram, pseudo-colors,
        color transfer function...

 */

//*************************************************************************

/*!
    Constructor
*/
HistogramRendererView::HistogramRendererView(QWidget *parent) :
    AbstractRendererView(parent),
    _ui(new Ui_HistogramRendererView),
//    _hRenderer(0),
    _colorPalette(0),
    _removeSliderAction(tr("Remove slider"), this),
    _addSliderAction(tr("Add slider"), this),
    _revertSliderAction(tr("Center color"), this),
    _indexOfActionedSlider(-1),
    _currentHistogram(0)
{
    _ui->setupUi(this);

    // MAKE ALL BANDS OPTION INVISIBLE
    _ui->_isAllBands->setVisible(false);
    // MAKE ALL BANDS OPTION INVISIBLE

    connect(_ui->_isPartial, SIGNAL(clicked(bool)), this, SLOT(onDisplayPartialHist(bool)));
    connect(_ui->_isComplete, SIGNAL(clicked(bool)), this, SLOT(onDisplayCompleteHist(bool)));
    connect(_ui->_histList, SIGNAL(activated(int)), this, SLOT(onHistListIndexChanged(int)));
    connect(_ui->_discreteColors, SIGNAL(clicked(bool)), this, SLOT(onDiscreteColorsClicked(bool)));
	connect(_ui->_revert, SIGNAL(clicked()), this, SLOT(resetToDefault()));
    connect(_ui->_transferFunction, SIGNAL(activated(QString)), this, SLOT(onTransferFunctionChanged(QString)));
    connect(_ui->_isAllBands, SIGNAL(clicked(bool)), this, SLOT(onIsAllBandsClicked(bool)));

    // setup timer:
    _updateDelayTimer.setSingleShot(true);
    connect(&_updateDelayTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerTimeout()));


    // setup context menu:
    setupMenu();

    // setup color picker
    _colorPicker.setWindowFlags(Qt::Popup);
    connect(&_colorPicker, SIGNAL(colorPicked(QColor)), this, SLOT(onColorPicked(QColor)));

    // setup value editor
    _valueEditor.setWindowFlags(Qt::Popup);
    _valueEditor.installEventFilter(this);
    _valueEditor.setAlignment(Qt::AlignRight);
    connect(&_valueEditor, SIGNAL(returnPressed()), this, SLOT(onValueEdited()));


    // Set scene size:
    _histogramScene.setSceneRect(-_settings.margin,
                                 -_settings.margin,
                                 1.0+2.0*_settings.margin,
                                 1.0+2.0*_settings.margin);
    _histogramScene.installEventFilter(this);

    _settings.histogramTransform =
            QTransform::fromScale(1.0,_settings.histOverPaletteRatio);

    _settings.colorPaletteTransform =
            QTransform::fromScale(1.0, 1.0 -_settings.histOverPaletteRatio) *
            QTransform::fromTranslate(0.0, _histogramScene.height()*_settings.histOverPaletteRatio);



    // Draw items:
    clear();

    // set histogram scene to the view
    _ui->_histogramView->setStyleSheet("background: dark grey");
    _ui->_histogramView->setScene(&_histogramScene);
    _ui->_histogramView->setRenderHint(QPainter::Antialiasing);


    // set transfer function names:
    setTransferFunctionNames(Core::HistogramRendererConfiguration::getAvailableTransferFunctionNames());

}

//*************************************************************************

/*!
    Destructor
*/
HistogramRendererView::~HistogramRendererView()
{
    delete _ui;
}

//*************************************************************************

/*!
    Method to clear all
*/
void HistogramRendererView::clear()
{
    _histograms.clear();
    _histogramScene.clear();
    _colorPalette = 0;
    _sliderLines.clear();
    _currentHistogram = 0;

    // Make frame 'toRGBMapping' invisible at init
    _ui->_toRGBMapping->setVisible(false);

    _ui->_isPartial->setEnabled(false);
    _ui->_isComplete->setEnabled(false);

    _ui->_histList->clear();
    _ui->_histList->setEnabled(false);

    _ui->_discreteColors->setEnabled(false);
    _ui->_discreteColors->setChecked(false);

    _ui->_revert->setEnabled(false);
    _ui->_transferFunction->setEnabled(false);

    _ui->_isAllBands->setEnabled(false);

    drawAxes();
    _ui->_histogramView->fitInView(_histogramScene.sceneRect());
}

//*************************************************************************

/*!
    Method to reset sliders to default values
*/
void HistogramRendererView::resetToDefault()
{
    if (!_currentHistogram)
        return;

    if (_colorPalette)
    {
        // remove color palette with sliders
        _histogramScene.removeItem(_colorPalette);
    }
    // redraw color palette & sliders & sliderlines
    createColorPalette();

    _ui->_transferFunction->setCurrentIndex(0);
    _currentHistogram->transferFunctionName=_ui->_transferFunction->currentIndex();
    _ui->_discreteColors->setChecked(false);
    _currentHistogram->isDiscrete=_ui->_discreteColors->isChecked();

    // reset current histogram outputstops
    if (_currentHistogram->bandId > -1)
    {
        int index = _ui->_histList->currentIndex();
        double a=(_currentHistogram->xmax2 - _currentHistogram->xmin2);
        double b=_currentHistogram->xmin2;
        if (_mode == RGB)
            _currentHistogram->outputStops = Core::resetStops(index,a,b);
        else
            _currentHistogram->outputStops = Core::resetStops(-1,a,b);
        // redraw histogram and sliders:
        drawHistogram(index);
    }
    else
    {
        double a=(_currentHistogram->xmax2 - _currentHistogram->xmin2);
        double b=_currentHistogram->xmin2;
        _currentHistogram->outputStops = Core::resetStops(4,a,b);
        // redraw
        drawAllHistograms();
    }


    // notify
    emit renderConfigurationChanged();

}

//*************************************************************************

void HistogramRendererView::initializeAllBandsHistogram()
{
    _allBandsHistogram.bandId=-1;
    double xmin(1e10), xmax(-1e10), xmin2(xmin), xmax2(xmax);
    foreach (HistogramDataView h, _histograms)
    {
        if (h.xmin < xmin) xmin = h.xmin;
        if (h.xmax > xmax) xmax = h.xmax;
        if (h.xmin2 < xmin2) xmin2 = h.xmin2;
        if (h.xmax2 > xmax2) xmax2 = h.xmax2;

    }
    _allBandsHistogram.xmin=xmin;
    _allBandsHistogram.xmin2=xmin2;
    _allBandsHistogram.xmax=xmax;
    _allBandsHistogram.xmax2=xmax2;
    _allBandsHistogram.vxmin = _allBandsHistogram.xmin;
    _allBandsHistogram.vxmax = _allBandsHistogram.xmax;
    _allBandsHistogram.isDiscrete=false;
    _allBandsHistogram.outputStops=Core::resetStops(4,xmax2-xmin2, xmin2);
    _allBandsHistogram.transferFunctionName=_ui->_transferFunction->itemText(0);
    _allBandsHistogram.graphicsItem=new QGraphicsItemGroup();
    _allBandsHistogram.graphicsItem->setVisible(true);
    _histogramScene.addItem(_allBandsHistogram.graphicsItem);

}

//*************************************************************************

/*!
    Method to setup context menu
*/
void HistogramRendererView::setupMenu()
{
    _removeSliderAction.setVisible(false);
    _addSliderAction.setVisible(false);
    _revertSliderAction.setVisible(false);
    _menu.addAction(&_removeSliderAction);
    _menu.addAction(&_addSliderAction);
    _menu.addAction(&_revertSliderAction);
    // connect actions:
    connect(&_removeSliderAction, SIGNAL(triggered()), this, SLOT(onRemoveSlider()));
    connect(&_addSliderAction, SIGNAL(triggered()), this, SLOT(onAddSlider()));
    connect(&_revertSliderAction, SIGNAL(triggered()), this, SLOT(onRevertSlider()));

}

//*************************************************************************
/*!
 * \brief HistogramRendererView::setup
 * \param renderer
 * \param layer
 */
void HistogramRendererView::setup(Core::ImageRenderer * renderer, const Core::ImageDataProvider * provider)
{

    setupRenderer(renderer);

    Core::HistogramImageRenderer* hRenderer = qobject_cast<Core::HistogramImageRenderer*>(_renderer);
    if (!hRenderer)
    {
        SD_TRACE("HistogramRendererView::setup : Failed to cast renderer into HistogramLayerRenderer");
        return;
    }

    Core::ImageRendererConfiguration conf = _renderer->getConfiguration();
    Core::HistogramRendererConfiguration histConf = hRenderer->getHistConfiguration();


    // configure toRGBMapping frame:
    int nbBands = provider->getNbBands();
    bool isComplex = provider->isComplex();
//    if (nbBands > 1 && !isComplex)
    if (histConf.mode == Core::HistogramRendererConfiguration::RGB)
    {
        _ui->_toRGBMapping->setVisible(true);
        configureAChannel(_ui->_redChannel, conf.toRGBMapping[0]+1, 1, nbBands);
        configureAChannel(_ui->_greenChannel, conf.toRGBMapping[1]+1, 1, nbBands);
        configureAChannel(_ui->_blueChannel, conf.toRGBMapping[2]+1, 1, nbBands);
        configureAChannel(_ui->_grayChannel, conf.toRGBMapping[0]+1, 1, nbBands);
    }
    else
//    if (nbBands == 1 && !isComplex)
    {
        _ui->_toRGBMapping->setVisible(false);
    }

    // create histogram views
    // 1) 1 band layer -> mode=GRAY, 1 histogram
    // 2) Complex N bands layer -> mode=GRAY, 1 of 4*N histograms
    // 3) Non-complex N bands layer -> { mode=RGB, 3 histograms | mode=GRAY, 1 of N histograms }

    nbBands = qMin(3, nbBands);
    for (int i=0; i<nbBands;i++)
    {
        int index = conf.toRGBMapping[i];
        addHistogram(index, provider->getBandNames()[index],
                     provider->getBandHistograms()[index],
                     conf, histConf);
    }

    drawHistogram();
}

//*************************************************************************
/*!
 * \brief HistogramRendererView::applyNewRendererConfiguration
 */
void HistogramRendererView::applyNewRendererConfiguration()
{
    Core::HistogramImageRenderer* hRenderer = qobject_cast<Core::HistogramImageRenderer*>(_renderer);
    if (!hRenderer)
    {
        SD_TRACE("HistogramRendererView::setup : Failed to cast renderer into HistogramLayerRenderer");
        return;
    }

    // RGB mapping configuration
    if (_mode == RGB)
    {
        Core::ImageRendererConfiguration conf = hRenderer->getConfiguration();
        conf.toRGBMapping[0] = _ui->_redChannel->value()-1;
        conf.toRGBMapping[1] = _ui->_greenChannel->value()-1;
        conf.toRGBMapping[2] = _ui->_blueChannel->value()-1;
        hRenderer->setConfiguration(conf);
    }

    // Histogram configuration
    int index = _currentHistogram->bandId;
    Core::HistogramRendererConfiguration histConf = hRenderer->getHistConfiguration();
    if (index > -1)
    {
        histConf.isDiscreteValues[index] = _currentHistogram->isDiscrete;
        histConf.normHistStops[index] = computeStopsFromValues(_currentHistogram->outputStops, _currentHistogram->xmin, _currentHistogram->xmax);
        histConf.transferFunctions[index] = Core::HistogramRendererConfiguration::getTransferFunctionByName(_currentHistogram->transferFunctionName);
    }
    else
    { // all bands -> update all histograms and histConf
        for (int i=0;i<_histograms.size();i++)
        {
            copyPositions(_currentHistogram->outputStops,_histograms[i].outputStops);
            histConf.isDiscreteValues[i]=_histograms[i].isDiscrete;
            histConf.normHistStops[i]=computeStopsFromValues(_histograms[i].outputStops, _histograms[i].xmin, _histograms[i].xmax);
            histConf.transferFunctions[i]=Core::HistogramRendererConfiguration::getTransferFunctionByName(_histograms[i].transferFunctionName);
        }
    }
    hRenderer->setHistConfiguration(histConf);
}

//*************************************************************************

/*!
    Method to add histogram data. For single band display, add only one histogram. For RGB display, add exactly 3 histograms.
*/
void HistogramRendererView::addHistogram(int id, const QString &name, const QVector<double> & data,
                                         const Core::ImageRendererConfiguration &conf, const Core::HistogramRendererConfiguration & histConf)
{
    _ui->_histList->addItem(name, id);

    int index = _histograms.size();
    _histograms.resize(index+1);
    _histograms[index].bandId=id;
    _histograms[index].xmin=conf.minValues[id];
    _histograms[index].xmax=conf.maxValues[id];
    _histograms[index].xmin2=histConf.qMinValues[id];
    _histograms[index].xmax2=histConf.qMaxValues[id];
    _histograms[index].transferFunctionName=histConf.transferFunctions[id]->getName();
    _histograms[index].isDiscrete=histConf.isDiscreteValues[id];
    _histograms[index].outputStops=computeValuesFromStops(histConf.normHistStops[id], _histograms[index].xmin, _histograms[index].xmax);
    // draw histogram bars :
    _histograms[index].graphicsItem = createHistogramGraphicsItem(data, _settings.dataPen);
    _histograms[index].graphicsItem->setVisible(false);
    _histogramScene.addItem(_histograms[index].graphicsItem);

}

//*************************************************************************

/*!
    Method to draw histogram in gray or rgb mode :
*/
void HistogramRendererView::drawHistogram()
{

    // Check if there is only one histogram added in the list:
    if (_histograms.size() == 1)
    {
        _mode = GRAY;
        _ui->_isAllBands->setEnabled(false);
    }
    else if (_histograms.size() == 3)
    {
        _mode = RGB;
        _ui->_isAllBands->setEnabled(true);
        initializeAllBandsHistogram();
    }
    else
    {
        SD_TRACE("HistogramRendererView::drawHistogram() : user should add 1 (gray mode) or 3 (rgb mode) histogram datas");
        return;
    }



    if (_ui->_transferFunction->count()==0)
    {
        // hide transfer function option
        _ui->_transferFunction->setVisible(false);
        _ui->_transferFunctionLabel->setVisible(false);
    }

    _ui->_histList->setEnabled(true);
    _ui->_histList->setCurrentIndex(0);
    _ui->_isPartial->setEnabled(true);
    _ui->_isPartial->setChecked(true);
    _ui->_isComplete->setEnabled(true);

    _ui->_discreteColors->setEnabled(true);
    _ui->_discreteColors->setChecked(false);

    _ui->_revert->setEnabled(true);

    _ui->_transferFunction->setEnabled(true);
    _ui->_transferFunction->setCurrentIndex(0);

    drawHistogram(0);

}

//*************************************************************************

/*!
    Method to set transfer function names
*/
void HistogramRendererView::setTransferFunctionNames(const QStringList &transferFunctionNames)
{
    _ui->_transferFunction->clear();
    foreach (QString tf, transferFunctionNames)
    {
        _ui->_transferFunction->addItem(tf);
    }

    // set visible if specified at least one transfer function
    if (!_ui->_transferFunction->isVisible())
    {
        _ui->_transferFunction->setVisible(true);
        _ui->_transferFunctionLabel->setVisible(true);
    }
}

//*************************************************************************

/*!
    Slot called when update time is timeout and one need send updated color schema
*/
void HistogramRendererView::onUpdateTimerTimeout()
{
    emit renderConfigurationChanged();
}

//*************************************************************************

/*!
    Method to update histogram data: stops and outputstops
*/
void HistogramRendererView::updateHistogramData(int index, double xpos, const QColor &c, bool notificationDelayed)
{
    if (!_currentHistogram)
        return;
    if (index < 0)
    {
        _currentHistogram->outputStops=_colorPalette->getPalette();
    }
    else if (index >= 0 && xpos > -12344)
    {
        _currentHistogram->outputStops[index].first = xpos;
    }
    else if (index >= 0 && c.isValid())
    {
        _currentHistogram->outputStops[index].second = c;
    }

    // notify about change:
    if (notificationDelayed)
        _updateDelayTimer.start(_settings.updateDelayMaxTime);
    else
    {
        emit renderConfigurationChanged();
    }

}

//*************************************************************************

/*!
    Slot to respond on slider position changed
*/
void HistogramRendererView::onSliderPositionChanged(int index, double position)
{
    if (index < 0 || index > _sliderLines.size() - 1 || !_currentHistogram)
        return;

    QGraphicsLineItem * sliderLine = _sliderLines[index];
    sliderLine->setTransform(_settings.histogramTransform);
    double npos=normalized(position, _currentHistogram->vxmin, _currentHistogram->vxmax);
    sliderLine->setTransform(QTransform::fromTranslate(npos, 0), true);

    // update histogram data
    updateHistogramData(index, position, QColor(), true);

}

//*************************************************************************

/*!
    Slot to handle add slider action
*/
void HistogramRendererView::onAddSlider()
{
    QPointF position = _addSliderAction.data().toPointF();
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
void HistogramRendererView::onRemoveSlider()
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
void HistogramRendererView::onColorPicked(QColor c)
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
void HistogramRendererView::onValueEdited()
{
    bool ok=false;
    double newvalue = _valueEditor.text().toDouble(&ok);
    if (ok)
    {
        int index = _indexOfActionedSlider;
        _indexOfActionedSlider=-1;
        if (index < 0 || index > _sliderLines.size() - 1)
            return;
        _colorPalette->setSliderValueAtIndex(index, newvalue);
        _valueEditor.hide();
        _colorPalette->highlightSliderTextAtIndex(index, false);

    }
}

//*************************************************************************

/*!
    Slot to handle slider revert action
*/
void HistogramRendererView::onRevertSlider()
{
    int index = _indexOfActionedSlider;
    _indexOfActionedSlider=-1;
    if (index < 0 || index > _sliderLines.size() - 1)
        return;

    _colorPalette->resetColorOfSliderAtIndex(index);

    updateHistogramData();

}

//*************************************************************************

/*!
    Slot to handle isComplete radio button clicked
*/
void HistogramRendererView::onDisplayCompleteHist(bool checked)
{
    if (!_currentHistogram || !checked)
        return;

    double newMin, newMax;
    newMin = _currentHistogram->xmin;
    newMax = _currentHistogram->xmax;

    transformAllItems(newMin, newMax);
}

//*************************************************************************

/*!
    Slot to handle the action that displays the histogram fitted to slider positions
*/
void HistogramRendererView::onDisplayPartialHist(bool checked)
{
    if (!_currentHistogram || !checked)
        return;

    double newMin, newMax;
    newMin = _currentHistogram->outputStops.first().first;
    newMax = _currentHistogram->outputStops.last().first;

    transformAllItems(newMin, newMax);
}

//*************************************************************************

/*!
    Slot to handle histogram list current index changed
*/
void HistogramRendererView::onHistListIndexChanged(int i)
{
    drawHistogram(i);
}

//*************************************************************************

/*!
    Slot to handle histogram list current index changed
*/
void HistogramRendererView::onTransferFunctionChanged(QString tf)
{
    if (!_currentHistogram || _currentHistogram->transferFunctionName == tf)
        return;

    _currentHistogram->transferFunctionName = tf;
    emit renderConfigurationChanged();
}

//*************************************************************************

/*!
    Slot to handle discrete colors option of the histogram
*/
void HistogramRendererView::onDiscreteColorsClicked(bool checked)
{
    _colorPalette->setIsDiscrete(checked);
    _currentHistogram->isDiscrete = _colorPalette->isDiscrete();
    emit renderConfigurationChanged();
}

//*************************************************************************

/*!
    Slot to handle when all bands option is checked/unchecked
*/
void HistogramRendererView::onIsAllBandsClicked(bool checked)
{
    if (checked)
    {
        _ui->_histList->setEnabled(false);
        _ui->_discreteColors->setEnabled(false);
        _ui->_transferFunction->setEnabled(false);
        drawAllHistograms();
    }
    else
    {
        _ui->_histList->setEnabled(true);
        _ui->_discreteColors->setEnabled(true);
        _ui->_transferFunction->setEnabled(true);

        // set invisible all histograms:
        for (int i=0;i<_histograms.size();i++)
        {
            _histograms[i].graphicsItem->setVisible(false);
            _histograms[i].xmin = _allBandsHistogram.xmin;
            _histograms[i].xmax = _allBandsHistogram.xmax;
            _allBandsHistogram.graphicsItem->removeFromGroup(_histograms[i].graphicsItem);
        }
        // draw
        drawHistogram(_ui->_histList->currentIndex());
    }

}

//*************************************************************************

/*!
    on Show event
*/
void HistogramRendererView::showEvent(QShowEvent * event)
{
    if (event->type() == QEvent::Show)
    {
        _ui->_histogramView->fitInView(_histogramScene.sceneRect());
    }
}

//*************************************************************************

/*!
    on Resize event
*/
void HistogramRendererView::resizeEvent(QResizeEvent * event)
{
    if (event->type() == QEvent::Resize)
    {
        _ui->_histogramView->fitInView(_histogramScene.sceneRect());
    }
}

//*************************************************************************

/*!
    Method to transform all graphics items
*/
void HistogramRendererView::transformAllItems(double newMin, double newMax)
{
    // transform histogram graphics item
    QTransform tr = _currentHistogram->graphicsItem->transform();
    updateToPartialMode(_currentHistogram->graphicsItem, _currentHistogram->vxmin, _currentHistogram->vxmax, newMin, newMax);
    _currentHistogram->graphicsItem->setTransform(tr, true);

    // transform slider lines
    foreach (QGraphicsLineItem * line, _sliderLines)
    {
        QTransform tr = line->transform();
        updateToPartialMode(line, _currentHistogram->vxmin, _currentHistogram->vxmax, newMin, newMax);
        line->setTransform(tr, true);
    }

    // transform slider inner positions
    _colorPalette->setMinMaxRanges(newMin, newMax);

    // update visual histogram range
    _currentHistogram->vxmin=newMin;
    _currentHistogram->vxmax=newMax;
}

//*************************************************************************

/*!
    Method to draw the histogram at index
*/
void HistogramRendererView::drawHistogram(int index)
{

    if (index < 0 || index >=_histograms.size())
        return;

    // make invisible current histogram:
    if (_currentHistogram && _currentHistogram->graphicsItem)
        _currentHistogram->graphicsItem->setVisible(false);

    HistogramDataView & h = _histograms[index];
    _currentHistogram = &h;

    drawHistogramGraphicsItem(&h, _settings.dataPen);

    // setup transfer function and isDiscrete coulours :
    _ui->_discreteColors->setChecked(_currentHistogram->isDiscrete);
    int i = _ui->_transferFunction->findText(_currentHistogram->transferFunctionName);
    if (i < 0)
    {
        i = 0;
        _currentHistogram->transferFunctionName=_ui->_transferFunction->itemText(i);
    }
    _ui->_transferFunction->setCurrentIndex(i);

    // draw color palette, sliders, etc in 100% view mode
    drawColorPalette(h.outputStops, h.vxmin, h.vxmax);

    // default choice is 95% of histogram to display
    _ui->_isPartial->setChecked(true);
    onDisplayPartialHist(true);

}

//*************************************************************************

/*!
    Method to draw all histograms
*/
void HistogramRendererView::drawAllHistograms()
{
    _currentHistogram = &_allBandsHistogram;
    // draw all histograms:
    QList<QColor> dataColors = QList<QColor>()
            << QColor(255,0,0,81)
            << QColor(0,255,0,81)
            << QColor(0,0,255,81);
    for (int i=0;i<_histograms.size();i++)
    {
        HistogramDataView & h = _histograms[i];
        drawHistogramGraphicsItem(&h, QPen(dataColors[i], 0.0));

        // set stops of the band histogram equal to the stops of allBands;
        copyPositions(_currentHistogram->outputStops,h.outputStops);
        _allBandsHistogram.graphicsItem->addToGroup(h.graphicsItem);
    }

    // draw color palette, sliders, etc in 100% view mode
    drawColorPalette(_currentHistogram->outputStops, _currentHistogram->vxmin, _currentHistogram->vxmax);

    // default choice is 95% of histogram to display
    _ui->_isPartial->setChecked(true);
    onDisplayPartialHist(true);


}

//*************************************************************************

/*!
    Method to create histogram graphics item
*/
QGraphicsItemGroup * HistogramRendererView::createHistogramGraphicsItem(const QVector<double> &data, const QPen &dataPen)
{
    // Draw histogram image : OX is [0.0; 1.0] / OY is [0.0, 1.0]
    QGraphicsItemGroup *group = new QGraphicsItemGroup();
    int histSize = data.size();
    for (int i=0; i<histSize; i++)
    {
        double value = data[i];
        QGraphicsLineItem * l = new QGraphicsLineItem(
                    i*1.0/histSize,
                    1.0 - value,
                    i*1.0/histSize,
                    1.0
                    );
        l->setPen(dataPen);
        group->addToGroup(l);
    }
    group->setZValue(0.0);
    return group;
}

//*************************************************************************

/*!
    Method to draw histogram graphics item
*/
QPen getHGIPen(QGraphicsItemGroup * hgi)
{
    QPen p;
    QList<QGraphicsItem*> items = hgi->childItems();
    if (items.isEmpty())
        return p;
    QGraphicsLineItem * line = qgraphicsitem_cast<QGraphicsLineItem *>(items[0]);
    if (line)
        p = line->pen();

    return p;

}

void HistogramRendererView::drawHistogramGraphicsItem(HistogramDataView * h, const QPen & dataPen)
{
    h->graphicsItem->setVisible(true);
    if (dataPen != getHGIPen(h->graphicsItem))
    {
        foreach (QGraphicsItem * item, h->graphicsItem->childItems())
        {
            QGraphicsLineItem * line = qgraphicsitem_cast<QGraphicsLineItem *>(item);
            if (line)
                line->setPen(dataPen);
        }
    }

    // intialize histogram bars transform and visible min max as 100 %
    h->vxmax = h->xmax;
    h->vxmin = h->xmin;
    h->graphicsItem->setTransform(_settings.histogramTransform);

}

//*************************************************************************

/*!
    Method to draw a color palette with n sliders from histogram info
    The map between line and slider is done with index in the _sliderLines and the index in the ColorPalette
*/
void HistogramRendererView::drawColorPalette(const QGradientStops & houtputStops, double hvxmin, double hvxmax)
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
        double xpos=normalized(houtputStops[i].first, hvxmin, hvxmax);
        sliderLine->setTransform(QTransform::fromTranslate(xpos, 0), true);
        sliderLine->setZValue(1.0);
        _sliderLines << sliderLine;
    }

    _colorPalette->setupPalette(houtputStops, hvxmin, hvxmax);

    connect(_colorPalette, SIGNAL(sliderPositionChanged(int,double)), this, SLOT(onSliderPositionChanged(int,double)));
}

//*************************************************************************

/*!
    Method to draw the axes
*/
void HistogramRendererView::drawAxes()
{
    // draw axes:
    QGraphicsItemGroup * axes = createAxesGroup(_settings.axisPen);
    _histogramScene.addItem(axes);
    axes->setTransform(_settings.histogramTransform);

    createColorPalette();

}

//*************************************************************************

/*!
    Method to create color palette graphics item
*/
void HistogramRendererView::createColorPalette()
{
    // draw color palette box
    _colorPalette = new ColorPalette;
    _histogramScene.addItem(_colorPalette);
    _colorPalette->setTransform(_settings.colorPaletteTransform);
}

//*************************************************************************

/*!
    Event filter :
     - to limit mouse interaction with sliders
*/
bool HistogramRendererView::eventFilter(QObject * object, QEvent * event)
{
    if (&_histogramScene == object && _histograms.size() > 0)
    {
        if (event->type() == QEvent::GraphicsSceneMousePress)
        {
            QGraphicsSceneMouseEvent* e = static_cast<QGraphicsSceneMouseEvent*>(event);
            QGraphicsItem * itemUnderMouse = _histogramScene.itemAt(e->scenePos(), QTransform());

            if (_colorPalette->itemIsSlider(itemUnderMouse) && _mode == GRAY )
            { // if clicked on slider :
                switch (e->button())
                {
                case Qt::RightButton:
                { // open context menu : - remove slider if nb(sliders) > 3
                    if (_colorPalette->getNbOfSliders() > 3)
                    {
                        _addSliderAction.setVisible(false);
                        _removeSliderAction.setVisible(true);
                        _revertSliderAction.setVisible(true);
                        _indexOfActionedSlider = _colorPalette->getSliderIndex(itemUnderMouse);
                        _menu.popup(e->screenPos());
                    }
                    else if (_colorPalette->getNbOfSliders() == 3)
                    {
                        _addSliderAction.setVisible(false);
                        _removeSliderAction.setVisible(false);
                        _revertSliderAction.setVisible(true);
                        _indexOfActionedSlider = _colorPalette->getSliderIndex(itemUnderMouse);
                        _menu.popup(e->screenPos());
                    }
                    e->accept();
                    return true;
                }
                }
            }
            else if (_colorPalette->itemIsPalette(itemUnderMouse) && _mode == GRAY)
            { // if clicked on color palette rectangle

                switch (e->button())
                {
                case Qt::RightButton:
                { // open context menu : - add slider

                    _removeSliderAction.setVisible(false);
                    _revertSliderAction.setVisible(false);
                    _addSliderAction.setVisible(true);
                    _addSliderAction.setData(
                                itemUnderMouse->mapFromScene(e->scenePos())
                                );
                    _menu.popup(e->screenPos());
                    e->accept();
                    return true;
                }
                }
            }

        }
        else if (event->type() == QEvent::GraphicsSceneMouseDoubleClick)
        {
            QGraphicsSceneMouseEvent* e = static_cast<QGraphicsSceneMouseEvent*>(event);
            QGraphicsItem * itemUnderMouse = _histogramScene.itemAt(e->scenePos(), QTransform());
            if (_colorPalette->itemIsSlider(itemUnderMouse) && _mode == GRAY)
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
    return QWidget::eventFilter(object, event);
}

//*************************************************************************

void HistogramRendererView::on__isGrayMode_toggled()
{
    SD_TRACE("on__isGrayMode_toggled");


}

//*************************************************************************

void HistogramRendererView::on__isRgbMode_toggled()
{
    SD_TRACE("on__isRgbMode_toggled");


}

//*************************************************************************

void HistogramRendererView::on__redChannel_editingFinished()
{
    int newValue = _ui->_redChannel->value()-1;

}

//*************************************************************************

void HistogramRendererView::on__greenChannel_editingFinished()
{
}

//*************************************************************************

void HistogramRendererView::on__blueChannel_editingFinished()
{
}

//*************************************************************************

} // namespace Gui

