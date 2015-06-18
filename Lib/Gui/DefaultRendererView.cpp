
// Project
#include "DefaultRendererView.h"
#include "Core/ImageDataProvider.h"

namespace Gui
{

//******************************************************************************

DefaultRendererView::DefaultRendererView(QWidget *parent) :
    AbstractRendererView(parent),
    ui(new Ui_DefaultRendererView)
{

    ui->setupUi(this);
    connect(ui->_revert, SIGNAL(clicked()), this, SLOT(revert()));

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

void DefaultRendererView::revert()
{
    setup(&_initialConf, _dataProvider);
    emit renderConfigurationChanged(&_initialConf);
}

//******************************************************************************

void DefaultRendererView::setup(const Core::ImageRendererConfiguration *conf, const Core::ImageDataProvider * provider)
{
    if (!provider || !conf)
    {
        SD_TRACE("HistogramRendererView::setup : provider or conf is null");
        return;
    }

    _conf = *conf;
    _initialConf = _conf;
    _dataProvider=provider;

    clear();

    ui->_band->addItems(_dataProvider->getBandNames());
    ui->_red->addItems(_dataProvider->getBandNames());
    ui->_green->addItems(_dataProvider->getBandNames());
    ui->_blue->addItems(_dataProvider->getBandNames());

    const QVector<int> & mapping = _conf.toRGBMapping;
    ui->_red->setCurrentIndex(mapping[0]);
    ui->_green->setCurrentIndex(mapping[1]);
    ui->_blue->setCurrentIndex(mapping[2]);

    // setup band conf
    setupBandConfiguration(0);

}

//******************************************************************************

void DefaultRendererView::setupBandConfiguration(int index)
{
    ui->_band->setCurrentIndex(index);
    ui->_min->setMinimum(_dataProvider->getMinValues()[index]);
    ui->_min->setMaximum(_dataProvider->getMaxValues()[index]);
    ui->_max->setMinimum(_dataProvider->getMinValues()[index]);
    ui->_max->setMaximum(_dataProvider->getMaxValues()[index]);

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
    emit renderConfigurationChanged(&_conf);
}

//******************************************************************************

void DefaultRendererView::on__green_activated(int index)
{
    _conf.toRGBMapping[1]=index;
    emit renderConfigurationChanged(&_conf);
}

//******************************************************************************

void DefaultRendererView::on__blue_activated(int index)
{
    _conf.toRGBMapping[2]=index;
    emit renderConfigurationChanged(&_conf);
}

//******************************************************************************

void DefaultRendererView::on__min_editingFinished()
{
    int index = ui->_band->currentIndex();
    _conf.minValues[index]=ui->_min->value();
    emit renderConfigurationChanged(&_conf);
}

//******************************************************************************

void DefaultRendererView::on__max_editingFinished()
{
    int index = ui->_band->currentIndex();
    _conf.maxValues[index]=ui->_max->value();
    emit renderConfigurationChanged(&_conf);
}

//******************************************************************************

}


