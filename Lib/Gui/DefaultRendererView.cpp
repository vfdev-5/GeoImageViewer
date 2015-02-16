
// Project
#include "DefaultRendererView.h"
#include "Core/ImageLayer.h"

namespace Gui
{

//******************************************************************************

DefaultRendererView::DefaultRendererView(QWidget *parent) :
    AbstractRendererView(parent),
    _renderer(0),
    ui(new Ui_DefaultRendererView)
{
    ui->setupUi(this);
}

//******************************************************************************

DefaultRendererView::~DefaultRendererView()
{
    delete ui;
}

//******************************************************************************

void DefaultRendererView::clear()
{
    ui->_red->clear();
    ui->_green->clear();
    ui->_blue->clear();
    ui->_band->clear();
    ui->_min->setValue(0.0);
    ui->_max->setValue(0.0);
}

//******************************************************************************

void DefaultRendererView::setup(Core::LayerRenderer * renderer, const Core::ImageLayer * layer)
{
    _renderer = renderer;
    _currentLayer=layer;

    clear();

    ui->_band->addItems(_currentLayer->getBandNames());
    ui->_red->addItems(_currentLayer->getBandNames());
    ui->_green->addItems(_currentLayer->getBandNames());
    ui->_blue->addItems(_currentLayer->getBandNames());

    // get rgb mapping:
    _conf = _renderer->getConfiguration();

    const QVector<int> & mapping = _conf.toRGBMapping;
    ui->_red->setCurrentIndex(mapping[0]);
    ui->_green->setCurrentIndex(mapping[1]);
    ui->_blue->setCurrentIndex(mapping[2]);

    // setup band conf
    setupBandConfiguration(0);

}

//******************************************************************************

void DefaultRendererView::applyNewRendererConfiguration()
{
    _renderer->setConfiguration(_conf);
}


//******************************************************************************

void DefaultRendererView::setupBandConfiguration(int index)
{
    ui->_band->setCurrentIndex(index);
    ui->_min->setMinimum(_currentLayer->getMinValues()[index]);
    ui->_min->setMaximum(_currentLayer->getMaxValues()[index]);
    ui->_max->setMinimum(_currentLayer->getMinValues()[index]);
    ui->_max->setMaximum(_currentLayer->getMaxValues()[index]);

    ui->_min->setValue(_conf.minValues[index]);
    ui->_max->setValue(_conf.maxValues[index]);
}

//******************************************************************************

void DefaultRendererView::on__band_activated(int index)
{
    setupBandConfiguration(index);
}

//******************************************************************************

void DefaultRendererView::on__red_activated(int index)
{
    _conf.toRGBMapping[0]=index;
    emit renderConfigurationChanged();
}

//******************************************************************************

void DefaultRendererView::on__green_activated(int index)
{
    _conf.toRGBMapping[1]=index;
    emit renderConfigurationChanged();
}

//******************************************************************************

void DefaultRendererView::on__blue_activated(int index)
{
    _conf.toRGBMapping[2]=index;
    emit renderConfigurationChanged();
}

//******************************************************************************

void DefaultRendererView::on__min_editingFinished()
{
    int index = ui->_band->currentIndex();
    _conf.minValues[index]=ui->_min->value();
    emit renderConfigurationChanged();
}

//******************************************************************************

void DefaultRendererView::on__max_editingFinished()
{
    int index = ui->_band->currentIndex();
    _conf.maxValues[index]=ui->_max->value();
    emit renderConfigurationChanged();
}

//******************************************************************************

}


