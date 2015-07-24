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

void setupStopsEndColor(QGradientStops & stops, const QColor &color)
{
    QGradientStop & stop = stops.last();
    stop.second = color;
}

void configureAChannel(QSpinBox * spinbox, int currentValue, int minValue, int maxValue)
{
    spinbox->setMinimum(minValue);
    spinbox->setMaximum(maxValue);    spinbox->setValue(currentValue);
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


    connect(_ui->_histogramView, SIGNAL(stopsChanged(int, QGradientStops)),
            this, SLOT(onStopsChanged(int, QGradientStops)));
    connect(_ui->_discreteColors, SIGNAL(clicked(bool)),
            _ui->_histogramView, SLOT(setDiscreteColors(bool)));

    connect(_ui->_revert, SIGNAL(clicked()), this, SLOT(revert()));

    // set transfer function names:
    setTransferFunctionNames(Core::HistogramRendererConfiguration::getAvailableTransferFunctionNames());

}

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

    _ui->_discreteColors->setEnabled(false);
    _ui->_discreteColors->setChecked(false);

    _ui->_revert->setEnabled(false);
    _ui->_transferFunction->setEnabled(false);

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
    _ui->_revert->setEnabled(true);
    _ui->_toRGBMapping->setEnabled(true);
    setRgbModeEnabled(nbBands > 1);

    if (_conf.mode == Core::HistogramRendererConfiguration::RGB)
    {

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

void HistogramRendererView::setupGrayModeView()
{
    _ui->_discreteColors->setEnabled(true);
    _ui->_transferFunction->setEnabled(true);

    _ui->_histogramView->clear();
    int nbBands = _dataProvider->getNbBands();

    // setup stops:
    if (_conf.mode == Core::HistogramRendererConfiguration::RGB)
    {
        if (!Core::HistogramImageRenderer::setupConfiguration(_dataProvider,
                                                             &_conf,
                                                             Core::HistogramRendererConfiguration::GRAY))
        {
            SD_TRACE("HistogramRendererView::setupGrayModeView : failed to setup configuration");
            return;
        }
    }

    for (int i=0;i<nbBands;i++)
    {
        _ui->_histogramView->addHistogram(_conf.normHistStops[i],
                                          _dataProvider->getBandHistograms()[i],
                                          _dataProvider->getMinValues()[i],
                                          _dataProvider->getMaxValues()[i],
                                          _conf.isDiscreteValues[i]);
    }

}

//*************************************************************************

void HistogramRendererView::setupRgbModeView()
{
    _ui->_discreteColors->setEnabled(false);
    _ui->_transferFunction->setEnabled(false);

    _ui->_histogramView->clear();
    // setup end stops colors:
    int indices[] = {_ui->_redChannel->value()-1,
                     _ui->_greenChannel->value()-1,
                     _ui->_blueChannel->value()-1};
    QColor colors[] = {QColor(Qt::red),
                       QColor(Qt::green),
                       QColor(Qt::blue)};


    // setup stops:
    if (_conf.mode == Core::HistogramRendererConfiguration::GRAY)
    {
        if (!Core::HistogramImageRenderer::setupConfiguration(_dataProvider,
                                                             &_conf,
                                                             Core::HistogramRendererConfiguration::RGB))
        {
            SD_TRACE("HistogramRendererView::setupGrayModeView : failed to setup configuration");
            return;
        }
    }

    for (int i=0;i<3;i++)
    {
        QGradientStops stops = _conf.normHistStops[indices[i]];
        setupStopsEndColor(stops, colors[i]);
        _ui->_histogramView->addHistogram(stops,
                _dataProvider->getBandHistograms()[indices[i]],
                _dataProvider->getMinValues()[indices[i]],
                _dataProvider->getMaxValues()[indices[i]],
                false);
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
    _ui->_discreteColors->setChecked(_conf.isDiscreteValues[index]);
    _ui->_transferFunction->setCurrentText(_conf.transferFunctions[index]->getName());
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

void HistogramRendererView::on__discreteColors_clicked(bool checked)
{
    SD_TRACE("on__discreteColors_clicked");
    if (_ui->_isGrayMode->isChecked())
    {
        int index = _ui->_grayChannel->value()-1;
        _conf.isDiscreteValues[index] = checked;
        emit renderConfigurationChanged(&_conf);
    }
}

//*************************************************************************

void HistogramRendererView::setRgbHistogram()
{
    _ui->_histogramView->drawRgbHistogram();
    _ui->_discreteColors->setChecked(false);
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

void HistogramRendererView::onStopsChanged(int hIndex, const QGradientStops & newstops)
{
    if (_ui->_isGrayMode->isChecked())
    {
        int index = _ui->_grayChannel->value()-1;
        if (index != hIndex)
        {
            SD_TRACE("HistogramRendererView::onStopsChanged : Indices are not equal");
            return;
        }
        _conf.normHistStops[index] = newstops;
        emit renderConfigurationChanged(&_conf);
    }
    else if (_ui->_isRgbMode->isChecked())
    {
        int index = _conf.toRGBMapping[hIndex];
        _conf.normHistStops[index] = newstops;
        emit renderConfigurationChanged(&_conf);
    }
}

//*************************************************************************

void HistogramRendererView::on__transferFunction_activated(QString text)
{
    if (_ui->_isGrayMode->isChecked())
    {
        int index = _ui->_grayChannel->value()-1;
        Core::TransferFunction * newFunction = Core::HistogramRendererConfiguration::getTransferFunctionByName(text);
        if (newFunction != _conf.transferFunctions[index])
        {
            _conf.transferFunctions[index] = newFunction;
            emit renderConfigurationChanged(&_conf);
        }
    }
}

//*************************************************************************

} // namespace Gui

