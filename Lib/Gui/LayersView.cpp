
// Qt
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QDropEvent>

// Project
#include "LayersView.h"
#include "ui_LayersView.h"
#include "Core/BaseLayer.h"


namespace Gui
{

//******************************************************************************

LayersView::LayersView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LayersView),
    _removeLayer(tr("Remove"), this),
    _saveLayer(tr("Save"), this),
    _createNewLayer(tr("Create"), this)
{
    ui->setupUi(this);

    ui->_editor->setNameFilter(QStringList()
                               << "objectName"
                               << "filePath"
                               << "fileToOpen"
                               << "isVisible"
                               );

    ui->_layers->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->_layers, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));
    connect(ui->_layers, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onItemChanged(QListWidgetItem*)));
    connect(ui->_layers, SIGNAL(itemOrderChanged()), this, SLOT(onItemOrderChanged()));
    connect(ui->_layers, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));


    connect(&_removeLayer, SIGNAL(triggered()), this, SLOT(onRemoveLayer()));
    connect(&_saveLayer, SIGNAL(triggered()), this, SLOT(onSaveLayer()));
    connect(&_createNewLayer, SIGNAL(triggered()), this, SIGNAL(createNewLayer()));

    _menu.addAction(&_createNewLayer);
    _menu.addSeparator();
    _menu.addAction(&_saveLayer);
    _menu.addSeparator();
    _menu.addAction(&_removeLayer);
}

//******************************************************************************

LayersView::~LayersView()
{
    delete ui;
}

//******************************************************************************

void LayersView::setupMenuOnItem()
{
    _createNewLayer.setEnabled(false);
    _removeLayer.setEnabled(true);
    _saveLayer.setEnabled(false);
}

//******************************************************************************

void LayersView::setupMenuNoItem()
{
    _createNewLayer.setEnabled(true);
    _removeLayer.setEnabled(false);
    _saveLayer.setEnabled(false);
}

//******************************************************************************

void LayersView::onRemoveLayer()
{
    QListWidgetItem * item = ui->_layers->currentItem();
    Core::BaseLayer * layer = _itemLayerMap.value(item, 0);

    // reset editor info if removed layer is current
    if (ui->_editor->getObject() == layer)
    {
//    QListWidgetItem * sItem = ui->_layers->selectedItems().first();
//    if (sItem == item)
        ui->_editor->setup(0);
    }

    if (layer)
    {
        _itemLayerMap.remove(item);
        delete item;
        delete layer;
        // signal is sent to ShapeViewer that layer is destroyed and a slot is called
    }
}

//******************************************************************************

void LayersView::onSaveLayer()
{

    QListWidgetItem * item = ui->_layers->currentItem();
    Core::BaseLayer * layer = _itemLayerMap.value(item, 0);
    if (layer)
        emit saveLayer(layer);

}

//******************************************************************************

void LayersView::addLayer(Core::BaseLayer *layer)
{
    int index = ui->_layers->count();
    QString layerName = tr("Layer %1 (%2)")
            .arg(index + 1)
            .arg(layer->getType());
    QListWidgetItem * item = new QListWidgetItem(layerName);
    item->setFlags(Qt::ItemIsSelectable |
                   Qt::ItemIsEditable |
                   Qt::ItemIsDragEnabled |
                   Qt::ItemIsDropEnabled |
                   Qt::ItemIsEnabled |
                   Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    layer->setZValue(-12345);

    _itemLayerMap.insert(item, layer);
    // insert item at row = 0
    ui->_layers->insertItem(0, item);

    ui->_layers->setCurrentItem(item);
    onItemClicked(item);

    updateZValues();


}

//******************************************************************************

void LayersView::setLayers(const QList<Core::BaseLayer *> &layers)
{
    ui->_layers->clear();
    foreach (Core::BaseLayer * layer, layers)
    {
        addLayer(layer);
    }

}

//******************************************************************************

Core::BaseLayer * LayersView::getCurrentLayer()
{
    QListWidgetItem * item = ui->_layers->currentItem();
    return _itemLayerMap.value(item, 0);
}

//******************************************************************************

void LayersView::onItemClicked(QListWidgetItem * item)
{

    Core::BaseLayer * layer = _itemLayerMap.value(item, 0);
    if (!layer)
    {
        SD_TRACE("onItemClicked : Layer is not found");
        ui->_editor->setup(0);
        return;
    }

    layer->setVisible(item->checkState() == Qt::Checked);

    ui->_editor->setup(layer);
    item->setSelected(true);

    emit layerSelected(layer);
}

//******************************************************************************

void LayersView::onItemChanged(QListWidgetItem * item)
{

    // !!! NOTHING TO DO !!!

//    int index = ui->_layers->row(item);
//    SD_TRACE(QString("onItemChanged : index=%1 text=%2")
//             .arg(index)
//             .arg(item->text()));
}

//******************************************************************************

void LayersView::onItemOrderChanged()
{
    updateZValues();
}

//******************************************************************************

void LayersView::updateZValues()
{
    int count = ui->_layers->count();
    for (int z=0; z < count; z++)
    {
        QListWidgetItem * item = ui->_layers->item(count - 1 - z);
//        int index = ui->_layers->row(item);
//        SD_TRACE(QString("item : index=%1 text=%2")
//                     .arg(index)
//                     .arg(item->text()));
        Core::BaseLayer * layer = _itemLayerMap.value(item, 0);
        if (!layer)
        {
            SD_TRACE("updateZValues : Layer is not found");
            continue;
        }
        if (layer->getZValue() != z)
        {
            layer->setZValue(z);
        }
    }
}

//******************************************************************************

void LayersView::onContextMenuRequested(QPoint p)
{
    QListWidgetItem * item = ui->_layers->itemAt(p);
    if (item)
    {
        setupMenuOnItem();
        // test if layer can be saved
        Core::BaseLayer * layer = _itemLayerMap.value(item, 0);
        if (layer && layer->canSave())
        {
            _saveLayer.setEnabled(true);
        }

        _menu.popup(ui->_layers->mapToGlobal(p));
    }
    else
    {
        setupMenuNoItem();
        _menu.popup(ui->_layers->mapToGlobal(p));
    }
}

//******************************************************************************
//******************************************************************************

void LayersListWidget::dropEvent(QDropEvent *e)
{
    int prevIndex = row(currentItem());
//    SD_TRACE("prev index : " + QString::number(prevIndex));
    QListWidget::dropEvent(e);
    int newIndex = row(currentItem());
//    SD_TRACE("new index : " + QString::number(newIndex));
    if (newIndex != prevIndex)
        emit itemOrderChanged();
}

//******************************************************************************

}
