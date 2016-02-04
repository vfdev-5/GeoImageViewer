// Qt
#include <QProgressDialog>


// Project
#include "DefaultFilterDialog.h"
#include "EditableFilterDialog.h"
#include "FilteringView.h"
#include "Filters/AbstractFilter.h"
#include "Filters/EditableFilter.h"
#include "Core/GeoImageLayer.h"
#include "Core/GeoImageItem.h"
#include "Core/ImageDataProvider.h"

namespace Gui
{

//******************************************************************************
/*!
* \class FilteringView
*
* \brief This is a view controller for layer filtering system
*/
//******************************************************************************

FilteringView::FilteringView(QProgressDialog *progress, QObject *parent) :
    QObject(parent),
    _filterDialog(0),
    _srcLayer(0),
    _dstLayer(0),
    _filter(0),
    _progressDialog(progress)
{
    //    connect(Filters::FiltersManager::get(),
    //            &Filters::FiltersManager::filteringFinished,
    //            this,
    //            &FilteringView::onFilteringFinished);
}

//******************************************************************************

FilteringView::~FilteringView()
{
    reset();
}

//******************************************************************************

void FilteringView::setup(Filters::AbstractFilter * f)
{
    if (!_srcLayer)
    {
        SD_ERR("Please, select an image layer before applying a filter");
        return;
    }

    SD_TRACE("Filter \'" + f->getName() + "\' is triggered");

    if (_filterDialog)
    {
        SD_TRACE("FilteringView::setup : Call reset() before");
        return;
    }

    Filters::EditableFilter * ef = qobject_cast<Filters::EditableFilter*>(f);
    if (!ef)
    {
        _filterDialog = new DefaultFilterDialog(f->getName() + tr(" dialog"));
        _filterDialog->setFilter(f);
    }
    else
    {
        _filterDialog = new EditableFilterDialog(ef);
    }
    _filterDialog->setLayerName(_srcLayer->getImageName());
    _filterDialog->installEventFilter(this);

    connect(_filterDialog, &DefaultFilterDialog::applyFilter, this, &FilteringView::onApplyFilter);

    _filter = f;
    _filterDialog->show();

}

//******************************************************************************

void FilteringView::reset()
{
    SD_TRACE("FilteringView : RESET");
    _srcLayer = 0;
    _dstLayer = 0;
    _filter = 0;
    if (_filterDialog)
    {
        _filterDialog->deleteLater();
        _filterDialog = 0;
    }
}

//******************************************************************************

void FilteringView::setSrcLayer(Core::GeoImageLayer *layer)
{
    _srcLayer = layer;
    if (_filterDialog)
        _filterDialog->setLayerName(_srcLayer->getImageName());
}

//******************************************************************************

void FilteringView::onApplyFilter()
{
    SD_TRACE("Apply filter");


    //    Filters::EditableFilter * ef = qobject_cast<Filters::EditableFilter*>(_appliedFilter);
    //    SD_TRACE1("Compute result ef(123)=%1", ef->computeResult(123));

    const Core::GeoImageItem * item = qgraphicsitem_cast<const Core::GeoImageItem*>(_srcLayer->getConstItem());
    if (!item)
    {
        SD_TRACE("onApplyEditableFilter : item is null");
        return;
    }

    // get data :
    const Core::ImageDataProvider * provider = item->getConstDataProvider();
    _filter->setNoDataValue(Core::ImageDataProvider::NoDataValue);

    _progressDialog->setLabelText(_filter->getName() + tr(". Processing ..."));
    _progressDialog->setValue(0);
    _progressDialog->show();

    Filters::FiltersManager::get()->applyFilterInBackground(_filter, provider);
}

//******************************************************************************

bool FilteringView::eventFilter(QObject * object, QEvent * event)
{
    if (object == _filterDialog)
    {
        if (event->type() == QEvent::Close)
        {
            reset();
        }
    }
    return QObject::eventFilter(object, event);
}

//******************************************************************************

}
