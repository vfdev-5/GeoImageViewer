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

//QGraphicsLineItem * createSliderLine()
//{
//    QGraphicsLineItem * line = new QGraphicsLineItem(
//                0.0,0.0,
//                0.0,1.0
//                );
//    line->setPen(QPen(QBrush(Qt::white), 0.0, Qt::DashLine));
//    line->setZValue(2.0);
//    return line;
//}

//QGraphicsItemGroup * createAxesGroup(const QPen & pen)
//{
//    QGraphicsItemGroup * axes = new QGraphicsItemGroup();
//    QGraphicsLineItem * axisX = new QGraphicsLineItem(
//                0.0, 1.0, 1.0, 1.0
//                );
//    axisX->setPen(pen);
//    axes->addToGroup(axisX);

//    QGraphicsLineItem * axisY = new QGraphicsLineItem(
//                0.0, 0.0, 0.0, 1.0
//                );
//    axisY->setPen(pen);
//    axes->addToGroup(axisY);
//    return axes;
//}

//inline double normalized(double x, double xmin, double xmax)
//{
//    return (x - xmin)/(xmax - xmin);
//}

//inline double unnormalized(double x, double xmin, double xmax)
//{
//    return x*(xmax - xmin) + xmin;
//}

//inline void updateToPartialMode(QGraphicsItem * hgi, double oldMin, double oldMax, double newMin, double newMax)
//{
//    hgi->setTransform(
//                QTransform::fromTranslate(-normalized(newMin, oldMin, oldMax),0) *
//                QTransform::fromScale((oldMax - oldMin)/(newMax - newMin),1)
//                );
//}

//inline void copyPositions(const QGradientStops & src, QGradientStops & dst)
//{
//    if (src.size() != dst.size()) return;
//    for(int i=0;i<src.size();i++)
//    {
//        dst[i].first = src[i].first;
//    }
//}

void setupStopsEndColor(QGradientStops & stops, const QColor &color)
{
    QGradientStop & stop = stops.last();
    stop.second = color;
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
    _ui(new Ui_HistogramRendererView)
{
    _ui->setupUi(this);

    // Draw items:
    clear();

}



//HistogramRendererView::HistogramRendererView(QWidget *parent) :
//    AbstractRendererView(parent),
//    _ui(new Ui_HistogramRendererView),
////    _hRenderer(0),
//    _colorPalette(0),
//    _removeSliderAction(tr("Remove slider"), this),
//    _addSliderAction(tr("Add slider"), this),
//    _revertSliderAction(tr("Center color"), this),
//    _indexOfActionedSlider(-1),
//    _currentHistogram(0)
//{
//    _ui->setupUi(this);

//    // MAKE ALL BANDS OPTION INVISIBLE
//    _ui->_isAllBands->setVisible(false);
//    // MAKE ALL BANDS OPTION INVISIBLE

//    connect(_ui->_isPartial, SIGNAL(clicked(bool)), this, SLOT(onDisplayPartialHist(bool)));
//    connect(_ui->_isComplete, SIGNAL(clicked(bool)), this, SLOT(onDisplayCompleteHist(bool)));
//    connect(_ui->_histList, SIGNAL(activated(int)), this, SLOT(onHistListIndexChanged(int)));
//    connect(_ui->_discreteColors, SIGNAL(clicked(bool)), this, SLOT(onDiscreteColorsClicked(bool)));
//	connect(_ui->_revert, SIGNAL(clicked()), this, SLOT(resetToDefault()));
//    connect(_ui->_transferFunction, SIGNAL(activated(QString)), this, SLOT(onTransferFunctionChanged(QString)));
//    connect(_ui->_isAllBands, SIGNAL(clicked(bool)), this, SLOT(onIsAllBandsClicked(bool)));

//    // setup timer:
//    _updateDelayTimer.setSingleShot(true);
//    connect(&_updateDelayTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerTimeout()));


//    // setup context menu:
//    setupMenu();

//    // setup color picker
//    _colorPicker.setWindowFlags(Qt::Popup);
//    connect(&_colorPicker, SIGNAL(colorPicked(QColor)), this, SLOT(onColorPicked(QColor)));

//    // setup value editor
//    _valueEditor.setWindowFlags(Qt::Popup);
//    _valueEditor.installEventFilter(this);
//    _valueEditor.setAlignment(Qt::AlignRight);
//    connect(&_valueEditor, SIGNAL(returnPressed()), this, SLOT(onValueEdited()));


//    // Set scene size:
//    _histogramScene.setSceneRect(-_settings.margin,
//                                 -_settings.margin,
//                                 1.0+2.0*_settings.margin,
//                                 1.0+2.0*_settings.margin);
//    _histogramScene.installEventFilter(this);

//    _settings.histogramTransform =
//            QTransform::fromScale(1.0,_settings.histOverPaletteRatio);

//    _settings.colorPaletteTransform =
//            QTransform::fromScale(1.0, 1.0 -_settings.histOverPaletteRatio) *
//            QTransform::fromTranslate(0.0, _histogramScene.height()*_settings.histOverPaletteRatio);



//    // Draw items:
//    clear();

//    // set histogram scene to the view
//    _ui->_histogramView->setStyleSheet("background: dark grey");
//    _ui->_histogramView->setScene(&_histogramScene);
//    _ui->_histogramView->setRenderHint(QPainter::Antialiasing);


//    // set transfer function names:
//    setTransferFunctionNames(Core::HistogramRendererConfiguration::getAvailableTransferFunctionNames());

//}

//*************************************************************************

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

    // Make frame 'toRGBMapping'
    _ui->_toRGBMapping->setEnabled(false);

    _ui->_isPartial->setEnabled(false);
    _ui->_isComplete->setEnabled(false);

    _ui->_histList->clear();
    _ui->_histList->setEnabled(false);

    _ui->_discreteColors->setEnabled(false);
    _ui->_discreteColors->setChecked(false);

    _ui->_revert->setEnabled(false);
    _ui->_transferFunction->setEnabled(false);

    _ui->_isAllBands->setEnabled(false);

    _ui->_histogramView->clear();
}

//*************************************************************************

/*!
    Method to clear all
*/
void HistogramRendererView::revert()
{
    setup(&_initialConf, _dataProvider);
    emit renderConfigurationChanged(&_initialConf);
}

////*************************************************************************

///*!
//    Method to reset sliders to default values
//*/
//void HistogramRendererView::resetToDefault()
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
//    emit renderConfigurationChanged(&_conf);

//}

////*************************************************************************

//void HistogramRendererView::initializeAllBandsHistogram()
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
 * \brief HistogramRendererView::setup
 * \param renderer
 * \param layer
 */
void HistogramRendererView::setup(const Core::ImageRendererConfiguration *conf, const Core::ImageDataProvider * provider)
{
    if (!provider)
    {
        SD_TRACE("HistogramRendererView::setup : provider is null");
        return;
    }

    const Core::HistogramRendererConfiguration * test = static_cast<const Core::HistogramRendererConfiguration*>(conf);
    if (!test)
    {
        SD_TRACE("HistogramRendererView::setup : Failed to cast ImageRendererConfiguration into HistogramRendererConfiguration");
        return;
    }
    _conf = *test;
    _initialConf = _conf;
    _dataProvider = provider;

    int nbBands = _dataProvider->getNbBands();

    // Setup spinboxes:
    configureAChannel(_ui->_grayChannel, _conf.toRGBMapping[0]+1, 1, nbBands);
    configureAChannel(_ui->_redChannel  , _conf.toRGBMapping[0]+1, 1, nbBands);
    configureAChannel(_ui->_greenChannel, _conf.toRGBMapping[1]+1, 1, nbBands);
    configureAChannel(_ui->_blueChannel , _conf.toRGBMapping[2]+1, 1, nbBands);
    // Enable UI :
    _ui->_toRGBMapping->setEnabled(true);
    setRgbModeEnabled(nbBands > 1);

    // create histogram views
    // 1) 1 band layer -> mode=GRAY, 1 histogram
    // 2) Complex M bands layer -> mode=GRAY, 1 of 4*M histograms
    // 3) Non-complex N bands layer -> { mode=RGB, 3 histograms | mode=GRAY, 1 of N histograms }
    if (_conf.mode == Core::HistogramRendererConfiguration::RGB)
    {

        //        for (int i=0; i<3; i++)
        //        {
        //            int index = _conf.toRGBMapping[i];
        //            _ui->_histogramView->addHistogram(_conf.normHistStops[index],
        //                                              _dataProvider->getBandHistograms()[index]);
        //        }
        _ui->_isRgbMode->setChecked(true);
        setupRgbModeView();
        _ui->_histogramView->drawRgbHistogram(_ui->_redChannel->value()-1,
                                              _ui->_greenChannel->value()-1,
                                              _ui->_blueChannel->value()-1);
    }
    else
    {
        _ui->_isGrayMode->setChecked(true);
        setupGrayModeView();
        _ui->_histogramView->drawSingleHistogram(_ui->_grayChannel->value()-1);
    }


}

//*************************************************************************

void HistogramRendererView::setupGrayModeView()
{
    _ui->_histogramView->clear();
    int nbBands = _dataProvider->getNbBands();
    for (int i=0;i<nbBands;i++)
    {
        _ui->_histogramView->addHistogram(_dataProvider->getBandHistograms()[i],
                                          _dataProvider->getMinValues()[i],
                                          _dataProvider->getMaxValues()[i]);
    }

}

//*************************************************************************

void HistogramRendererView::setupRgbModeView()
{
    _ui->_histogramView->clear();
    // setup end stops colors:
    int indices[] = {_ui->_redChannel->value()-1,
                     _ui->_greenChannel->value()-1,
                     _ui->_blueChannel->value()-1};
    QColor colors[] = {QColor(Qt::red),
                       QColor(Qt::green),
                       QColor(Qt::blue)};

    for (int i=0; i<3; i++)
    {
        QGradientStops stops = _conf.normHistStops[indices[i]];
        setupStopsEndColor(stops, colors[i]);
        _ui->_histogramView->addHistogram(_dataProvider->getBandHistograms()[indices[i]],
                                          _dataProvider->getMinValues()[indices[i]],
                                          _dataProvider->getMaxValues()[indices[i]]);
    }
}

//*************************************************************************

void HistogramRendererView::setRgbModeEnabled(bool value)
{
    _ui->_isRgbMode->setEnabled(value);
    _ui->_redChannel->setEnabled(value);
    _ui->_greenChannel->setEnabled(value);
    _ui->_blueChannel->setEnabled(value);
}

//*************************************************************************

void HistogramRendererView::on__isGrayMode_clicked(bool checked)
{
    SD_TRACE("on__isGrayMode_clicked");
    if (checked)
    {
        setupGrayModeView();
        setGrayHistogram(_ui->_grayChannel->value()-1);
    }
}

//*************************************************************************

void HistogramRendererView::setGrayHistogram(int index)
{
    _ui->_histogramView->drawSingleHistogram(index);
    // setup rendering configuration
    _conf.toRGBMapping[0]=index;
    _conf.toRGBMapping[1]=index;
    _conf.toRGBMapping[2]=index;
    emit renderConfigurationChanged(&_conf);
}

//*************************************************************************

void HistogramRendererView::on__isRgbMode_clicked(bool checked)
{
    SD_TRACE("on__isRgbMode_clicked");

    if (checked)
    {
        // need to setup histogram data:
        setupRgbModeView();
        setRgbHistogram();
    }
}

//*************************************************************************

void HistogramRendererView::setRgbHistogram()
{
    _ui->_histogramView->drawRgbHistogram();
    // setup rendering configuration
    _conf.toRGBMapping[0]=_ui->_redChannel->value()-1;
    _conf.toRGBMapping[1]=_ui->_greenChannel->value()-1;
    _conf.toRGBMapping[2]=_ui->_blueChannel->value()-1;
    emit renderConfigurationChanged(&_conf);
}

//*************************************************************************

void HistogramRendererView::on__redChannel_editingFinished()
{
    if (_ui->_isRgbMode->isChecked())
    {
        setupRgbModeView();
        setRgbHistogram();
    }
}

//*************************************************************************

void HistogramRendererView::on__greenChannel_editingFinished()
{
    if (_ui->_isRgbMode->isChecked())
    {
        setupRgbModeView();
        setRgbHistogram();
    }
}

//*************************************************************************

void HistogramRendererView::on__blueChannel_editingFinished()
{
    if (_ui->_isRgbMode->isChecked())
    {
        setupRgbModeView();
        setRgbHistogram();
    }
}

//*************************************************************************

void HistogramRendererView::on__grayChannel_editingFinished()
{
    if (_ui->_isGrayMode->isChecked())
    {
        setGrayHistogram(_ui->_grayChannel->value()-1);
    }
}

//*************************************************************************

} // namespace Gui

