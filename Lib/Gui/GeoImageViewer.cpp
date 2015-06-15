#include "GeoImageViewer.h"

// Qt
#include <qmath.h>
#include <QProgressDialog>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QFileDialog>

// Project
#include "GeoImageViewer.h"
#include "AbstractRendererView.h"
#include "SubdatasetDialog.h"
#include "NewLayerDialog.h"
#include "DefaultFilterDialog.h"
#include "LayersView.h"
#include "AbstractToolsView.h"
#include "Core/Global.h"
#include "Core/ImageOpener.h"
#include "Core/ImageWriter.h"
#include "Core/GeoImageItem.h"
#include "Core/GeoImageLayer.h"
#include "Core/ImageDataProvider.h"
#include "Core/FloatingDataProvider.h"
#include "Core/HistogramImageRenderer.h"
#include "Core/LayerUtils.h"
#include "Core/DrawingsItem.h"
#include "Tools/SelectionTool.h"
#include "Tools/BrushTool.h"
#include "Tools/ThresholdFilterTool.h"
#include "Tools/ToolsManager.h"
#include "Filters/FiltersManager.h"


namespace Gui
{

//******************************************************************************
/*!
* \class GeoImageViewer
*
* \brief This is a widget that displays image. Reimplements ShapeViewer class
*   - Its method loadImage() with image path loads image.
*   - Method clear() restarts the viewer.
*   - Imagery is then represented by classes Image and ImageLayer that are stored in the map of
*     the singleton ImageManager
*/
//******************************************************************************

GeoImageViewer::GeoImageViewer(QWidget *parent) :
    ShapeViewer(tr("Drag and drop an image"), parent),
    _rendererView(0),
    _imageOpener(new Core::ImageOpener(this)),
    _imageWriter(new Core::ImageWriter(this)),
    _processedLayer(0),
    _appliedFilter(0)
{

    // Init scene and loader
    clear();
    connect(_imageOpener, SIGNAL(imageOpened(Core::ImageDataProvider*)), this, SLOT(onImageOpened(Core::ImageDataProvider *)));
    connect(_imageOpener, SIGNAL(openProgressValueChanged(int)), this, SLOT(onProgressValueChanged(int)));
    connect(_imageWriter, SIGNAL(imageWriteFinished(bool)), this, SLOT(onImageWriteFinished(bool)));
    connect(_imageWriter, SIGNAL(writeProgressValueChanged(int)), this, SLOT(onProgressValueChanged(int)));



    setMinimumSize(QSize(450, 450));

    // Create SelectionTool :
    Tools::SelectionTool * selection = new Tools::SelectionTool();
    _toolsManager->insertTool(selection);
    connect(selection, SIGNAL(copyToNewLayer(QRectF)), this, SLOT(onCopyData(QRectF)));

    // Create BrushTool :
    _toolsManager->insertTool(new Tools::BrushTool(&_scene, &_view));

    // Create ThresholdFilterTool :
    Tools::ThresholdFilterTool * thresholdTool = new Tools::ThresholdFilterTool(0, 0);
    _toolsManager->insertTool(thresholdTool);

    initFilterTools();

    // Connect filters:
    connect(Filters::FiltersManager::get(), SIGNAL(filteringFinished(Core::ImageDataProvider*)),
            this, SLOT(onFilteringFinished(Core::ImageDataProvider*)));
    connect(Filters::FiltersManager::get(), SIGNAL(filterProgressValueChanged(int)),
            this, SLOT(onProgressValueChanged(int)));

}

//******************************************************************************

GeoImageViewer::~GeoImageViewer()
{
}

//******************************************************************************

void GeoImageViewer::clear()
{
    SD_TRACE("GeoImageViewer::clear");
    if (_rendererView)
    {
        _rendererView->clear();
    }

    ShapeViewer::clear();
    enableOptions(false);
    showPointInfo(false);
}

//******************************************************************************

bool GeoImageViewer::onSceneDragAndDrop(const QList<QUrl> &urls)
{
    foreach (QUrl url, urls)
    {
        SD_TRACE("Drop : " + url.toEncoded());
        loadImage(url);
        break;
    }
    return true;
}

//******************************************************************************

void GeoImageViewer::loadImage(const QUrl & url)
{
    // reset previous state
    clear();

    _progressDialog->setLabelText("Load image ...");
    _progressDialog->setValue(0);
    _progressDialog->show();

    if (!_imageOpener->openImageInBackground(url))
    {
        _progressDialog->close();
    }

}

//******************************************************************************

void GeoImageViewer::prepareSceneAndView(int w, int h)
{
    enableOptions(true);
    // remove initial text and set background to black
    _scene.removeItem(_initialTextItem);
    delete _initialTextItem;
    // set visible whole image + boundaries:

    _scene.setSceneRect(
                QRectF(0,0,w,h)
                .adjusted(-0.25*w, -0.25*h, 0.25*w, 0.25*h)
                );
    _view.setBackgroundBrush(QBrush(Qt::black));
    showPointInfo(true);

}

//******************************************************************************

void GeoImageViewer::onImageOpened(Core::ImageDataProvider *imageDataProvider)
{
    if (!imageDataProvider)
    {
        SD_ERR("Application failed to read the image data");
        _progressDialog->close();
        return;
    }

    if (_layers.isEmpty())
    {
        int w = imageDataProvider->getWidth();
        int h = imageDataProvider->getHeight();
        prepareSceneAndView(w, h);
    }

    // Create Geo Image Item from data provider
    Core::GeoImageItem * item = createGeoImageItem(imageDataProvider);

    // Create Geo image layer and add it to layers storage
    Core::GeoImageLayer * layer = createGeoImageLayer("Image", imageDataProvider);

    addLayer(layer, item);

//    // configure renderer view :
//    if (_rendererView)
//    {
//        _rendererView->clear();
//        _rendererView->setup(item->getRenderer(), item->getConstDataProvider());
//    }

    // init display item in Scene
    _zoomMinLevel = item->getZoomMinLevel();
    centerOnAtZoom(_zoomMinLevel, _scene.sceneRect().center());


}

//******************************************************************************

void GeoImageViewer::onImageWriteFinished(bool ok)
{
    if (ok)
    {
        SD_INFO(tr("File is written successfully"));
    }
    else
    {
        SD_ERR(tr("Failed to write file"));
    }
}

//******************************************************************************

void GeoImageViewer::onProgressCanceled()
{
    SD_TRACE("GeoImageViewer::onProgressCanceled");
    if (_imageOpener->isWorking())
    {
        _imageOpener->cancel();
    }
    else
    if (_imageWriter->isWorking())
    {
        _imageWriter->cancel();
    }
    _processedLayer = 0;
    _appliedFilter = 0;
}

//******************************************************************************

void GeoImageViewer::onBaseLayerSelected(Core::BaseLayer * layer)
{
    SD_TRACE("GeoImageViewer::onBaseLayerSelected");
    ShapeViewer::onBaseLayerSelected(layer);

    // setup renderer view if layer is geo image layer
    Core::GeoImageItem * imItem = getGeoImageItem(layer);
    if (imItem)
    {
        if (_rendererView)
        {
            _rendererView->clear();
            _rendererView->setup(imItem->getRendererConfiguration(), imItem->getConstDataProvider());

            // disconnect everything connected to a specific signal
            _rendererView->disconnect(SIGNAL(renderConfigurationChanged(Core::ImageRendererConfiguration)));
            // connect the signal with current GeoImageItem
            connect(_rendererView, SIGNAL(renderConfigurationChanged(Core::ImageRendererConfiguration)),
                    imItem, SLOT(onRendererConfigurationChanged(Core::ImageRendererConfiguration)));
        }
    }

    // configure current tool :
    configureTool(_currentTool, layer);
//    configureTool(_currentTool, getCurrentLayer());

}

//******************************************************************************

void GeoImageViewer::onBaseLayerDestroyed(QObject * layerObj)
{
    ShapeViewer::onBaseLayerDestroyed(layerObj);
    configureTool(_currentTool, 0);

    // clear renderer view when layer and geo image item are destroyed
    if (_rendererView)
    {
        _rendererView->clear();
        // disconnect everything connected to a specific signal
        _rendererView->disconnect(SIGNAL(renderConfigurationChanged()));
    }

}

//******************************************************************************

void GeoImageViewer::onSaveBaseLayer(Core::BaseLayer * layer)
{
    if (qobject_cast<Core::GeoImageLayer*>(layer))
    {
        writeGeoImageLayer(layer);
    }
}

//******************************************************************************

void GeoImageViewer::onCreateBaseLayer()
{
    NewLayerDialog d;
    if (!_layers.isEmpty())
        d.setExtent(getVisibleSceneRect().toRect());

    if (d.exec())
    {
        QString name = d.getName();
        QRect r = d.getExtent();

        if (_layers.isEmpty())
        {
            prepareSceneAndView(r.width(), r.height());
        }

        // Create empty image data provider :
//        Core::FloatingDataProvider * provider = Core::FloatingDataProvider::createEmptyDataProvider(name, r.width(), r.height());
        // Create geo image item :
//        Core::GeoImageItem * item = createGeoImageItem(provider, r.topLeft());
        // connect on data changed :
//        connect(provider, SIGNAL(dataChanged(QRect)), item, SLOT(onDataChanged(QRect)));
        // Create geo image layer :
//        Core::BaseLayer * layer = createGeoImageLayer("Image", provider);


        Core::DrawingsItem * item = new Core::DrawingsItem(r.width(), r.height());
        item->setPos(r.x(), r.y());
        item->setZValue(1000);

        _scene.addItem(item);

        Core::GeoShapeLayer * layer = new Core::GeoShapeLayer(this);
        layer->setType("Scribble : " + name);
        layer->setEditable(true);
        layer->setPixelExtent(r);

        addLayer(layer, item);
    }
}

//******************************************************************************

void GeoImageViewer::writeGeoImageLayer(Core::BaseLayer * layer)
{
    _processedLayer = qobject_cast<Core::GeoImageLayer*>(layer);
    Core::GeoImageItem * item = static_cast<Core::GeoImageItem*>(_layerItemMap.value(_processedLayer, 0));
    if (!item)
    {
        SD_TRACE("GeoImageViewer::writeGeoImageLayer : item is null");
        return;
    }

    const Core::ImageDataProvider * provider = item->getConstDataProvider();

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save into a file"),
                                                    QString(),
                                                    tr("Images (*.tif)"));

    if (filename.isEmpty())
        return;

    _progressDialog->setLabelText("Save image ...");
    _progressDialog->setValue(0);
    _progressDialog->show();

    if (!_imageWriter->writeInBackground(filename, provider, _processedLayer))
    {
        _progressDialog->close();
    }
}

//******************************************************************************

void GeoImageViewer::onToolChanged(const QString & toolName)
{
    ShapeViewer::onToolChanged(toolName);
    configureTool(_currentTool, getCurrentLayer());
}

//******************************************************************************

void GeoImageViewer::onFilterTriggered()
{
    filterGeoImageLayer(getCurrentLayer());
}

//******************************************************************************

void GeoImageViewer::filterGeoImageLayer(Core::BaseLayer * layer)
{
    Core::GeoImageLayer * iLayer = qobject_cast<Core::GeoImageLayer*>(layer);
    Core::GeoImageItem * item = static_cast<Core::GeoImageItem*>(_layerItemMap.value(iLayer, 0));
    if (!iLayer || !item)
    {
        SD_ERR("Please, select an image layer before applying a filter");
        return;
    }

    QAction * a = qobject_cast<QAction*>(sender());
    if (!a)
    {
        SD_TRACE("GeoImageViewer::filterGeoImageLayer : failed to get qaction");
        return;
    }

    Filters::AbstractFilter * f = qobject_cast<Filters::AbstractFilter*>(a->data().value<QObject*>());
    if (!f)
    {
        SD_TRACE("GeoImageViewer::filterGeoImageLayer : failed to get filter");
        return;
    }

    SD_TRACE("Filter \'" + f->getName() + "\' is triggered");

    // Show Filter dialog
    DefaultFilterDialog d(f->getName() + tr(" dialog"));
    d.setFilter(f);
    if (d.exec())
    {
        _processedLayer = iLayer;
        _appliedFilter = f;
        SD_TRACE("Apply filter \'" + f->getName() + "\'");

        // get data :
        const Core::ImageDataProvider * provider = item->getConstDataProvider();

        f->setNoDataValue(Core::ImageDataProvider::NoDataValue);

        _progressDialog->setLabelText(f->getName() + tr(". Processing ..."));
        _progressDialog->setValue(0);
        _progressDialog->show();

        Filters::FiltersManager::get()->applyFilterInBackground(f, provider);
    }
}

//******************************************************************************

void GeoImageViewer::onFilteringFinished(Core::ImageDataProvider * provider)
{
    SD_TRACE("GeoImageViewer::onFilteringFinished");
    if (!provider)
    {
        SD_TRACE("GeoImageViewer::onFilteringFinished : provider is null");
        _progressDialog->close();
        return;
    }

    Core::GeoImageItem * item = static_cast<Core::GeoImageItem*>(_layerItemMap.value(_processedLayer, 0));
    if (!item)
    {
        SD_TRACE("GeoImageViewer::onFilteringFinished : item is null . something wrong");
        return;
    }

    QPointF pos = item->pos();
    Core::GeoImageItem * nItem = createGeoImageItem(provider, pos);

    // Create new layer :
    Core::GeoImageLayer * nLayer = new Core::GeoImageLayer(this);
    nLayer->setType(_appliedFilter->getName());
    nLayer->setImageName(provider->getImageName());

    nLayer->setNbBands(provider->getNbBands());
    nLayer->setDepthInBytes(provider->getDepthInBytes());
    nLayer->setIsComplex(provider->isComplex());

    nLayer->setGeoExtent(_processedLayer->getGeoExtent());
    nLayer->setGeoBBox(_processedLayer->getGeoBBox());
    // pixel extent is intersection
    nLayer->setPixelExtent(_processedLayer->getPixelExtent());
    nLayer->setProjectionRef(_processedLayer->getProjectionRef());

    addLayer(nLayer, nItem);

    // display item in Scene:
    nItem->updateItem(_zoomLevel, getVisibleSceneRect());

    _appliedFilter = 0;
    _processedLayer = 0;
}

//******************************************************************************
/*!
 * \brief GeoImageViewer::setRendererView method to setup a renderer view: DefaultRendererView or HistogramRendererView ...
 * \param rendererView
 */
void GeoImageViewer::setRendererView(AbstractRendererView *rendererView)
{
    _rendererView = rendererView;
//    connect(_rendererView, SIGNAL(renderConfigurationChanged()), this, SLOT(onRenderConfigurationChanged()));
}

//******************************************************************************

void GeoImageViewer::onCopyData(const QRectF &selection)
{
    SD_TRACE("Copy data to new layer");

    Core::GeoImageLayer * iLayer = qobject_cast<Core::GeoImageLayer*>(getCurrentLayer());
    if (!iLayer)
    {
        SD_ERR("Please, select an image layer");
        return;
    }
    // static cast is ensured with non null conversion of BaseLayer into GeoImageLayer
    Core::GeoImageItem * item = static_cast<Core::GeoImageItem*>(_layerItemMap.value(iLayer, 0));
    if (!item)
    {
        SD_ERR("Please, select an image layer");
        return;
    }

    const Core::ImageDataProvider * provider = item->getConstDataProvider();

    QRect pixelExtent = selection.translated(-item->pos().x(),
                                             -item->pos().y()
                                             ).toRect();

    // Create floating data provider :
    QRect srcPixelExtent = provider->getPixelExtent();
    QRect intersection = srcPixelExtent.intersected(pixelExtent);
    if (intersection.isEmpty())
    {
        SD_ERR("Layer does not contain data in the selection");
        return;
    }

    Core::FloatingDataProvider * nProvider = Core::FloatingDataProvider::createDataProvider(provider, intersection);

    if (!nProvider)
    {
        SD_TRACE("onCopyData : new data provider is null");
        return;
    }

    // Create Geo Image Item from data provider
    Core::GeoImageItem * nItem = createGeoImageItem(nProvider, selection.topLeft());

    // Create new layer :
    Core::GeoImageLayer * nLayer = createGeoImageLayer("Floating Image", nProvider, intersection);

    addLayer(nLayer, nItem);

    // display item in Scene:
    nItem->updateItem(_zoomLevel, getVisibleSceneRect());
}

//******************************************************************************

void GeoImageViewer::createScribble(const QString &name, Core::DrawingsItem *item)
{
    Core::GeoShapeLayer * layer = new Core::GeoShapeLayer(this);
    layer->setType("Scribble : " + name);
    layer->setEditable(true);
    layer->setPixelExtent(QRect(item->pos().toPoint(), item->boundingRect().size().toSize()));

    addLayer(layer, item);
}

//******************************************************************************

void GeoImageViewer::initFilterTools()
{
    foreach (Tools::AbstractTool * tool, _toolsManager->getTools())
    {
        if (tool->getType() == Tools::FilterTool::Type)
        {
            Tools::FilterTool * ftool = qobject_cast<Tools::FilterTool*>(tool);
            if (ftool)
            {
                ftool->setGraphicsSceneAndView(&_scene, &_view);
                connect(ftool, SIGNAL(drawingsFinalized(const QString&, Core::DrawingsItem*)), this, SLOT(createScribble(const QString&, Core::DrawingsItem*)));
                connect(ftool, SIGNAL(itemCreated(QGraphicsItem*)), this, SLOT(onItemCreated(QGraphicsItem*)));
            }
        }
    }
}

//******************************************************************************

bool GeoImageViewer::configureTool(Tools::AbstractTool *tool, Core::BaseLayer *layer)
{
    if (tool->getType() == Tools::ImageCreationTool::Type)
    {
        Tools::ImageCreationTool * ictool = qobject_cast<Tools::ImageCreationTool *>(tool);
        if (!ictool)
        {
            SD_TRACE("configureTool : Failed to get image creational tool");
            return false;
        }
        ictool->setDrawingsItem(0);
        if (!layer)
        {
            return false;
        }
        if (!layer->isEditable())
        {
            SD_WARN(tr("Please choose an editable layer or create a new one"));
            return false;
        }
        Core::GeoShapeLayer * scribble = qobject_cast<Core::GeoShapeLayer*>(layer);
        if (!scribble)
        {
            return false;
        }
        // static cast is ensured with non null conversion of BaseLayer into GeoShapeLayer
        Core::DrawingsItem * item = static_cast<Core::DrawingsItem*>(_layerItemMap.value(scribble, 0));
        if (!item)
        {
            SD_TRACE("onToolChanged : Failed to get geo image item");
            return false;
        }
        ictool->setDrawingsItem(item);
    }
    else if (tool->getType() == Tools::FilterTool::Type)
    {
        Tools::FilterTool * ftool = qobject_cast<Tools::FilterTool*>(tool);
        if (!ftool)
        {
            SD_TRACE("configureTool : Failed to get filter tool");
            return false;
        }
        ftool->setDataProvider(0);
        if (!layer)
        {
            return false;
        }

        Core::GeoImageItem* item = getGeoImageItem(layer);
        if (!item)
        {
            return false;
        }

        const Core::ImageDataProvider * data = item->getConstDataProvider();
        if (!data)
        {
            SD_TRACE("configureTool : Failed to get Image Data Provider");
            return false;
        }
        ftool->setDataProvider(data);

    }
    return true;
}

//******************************************************************************

Core::GeoImageItem * GeoImageViewer::createGeoImageItem(Core::ImageDataProvider * provider, const QPointF & pos)
{
    // Create histogram renderer:
    Core::HistogramImageRenderer * renderer =  new Core::HistogramImageRenderer();
    // Create renderer conf:
    Core::HistogramRendererConfiguration * rconf = new Core::HistogramRendererConfiguration();
    Core::HistogramImageRenderer::setupConfiguration(provider, rconf);

    // Create geo image item :
    Core::GeoImageItem * out = new Core::GeoImageItem(provider, renderer, rconf);
    out->setPos(pos);
    out->setZValue(1000);
    _scene.addItem(out);
    connect(this, SIGNAL(viewportChanged(int,QRectF)),
            out, SLOT(updateItem(int, QRectF)));

    return out;
}

//******************************************************************************
/*!
 * \brief GeoImageViewer::createGeoImageLayer to create geo image layer from data provider
 * \param type
 * \param provider
 * \param userPixelExtent is used to setup manually shown layer's extent. Otherwise, provider' extent is used.
 * \return geo image layer is returned
 */
Core::GeoImageLayer * GeoImageViewer::createGeoImageLayer(const QString &type, Core::ImageDataProvider * provider, const QRect & userPixelExtent)
{
    Core::GeoImageLayer * layer = new Core::GeoImageLayer(this);
    layer->setType(type);
    layer->setEditable(provider->isEditable());
    layer->setImageName(provider->getImageName());
    layer->setLocation(provider->getLocation());
    layer->setNbBands(provider->getInputNbBands());
    layer->setDepthInBytes(provider->getInputDepthInBytes());
    layer->setIsComplex(provider->inputIsComplex());

    layer->setGeoExtent(provider->fetchGeoExtent());
    layer->setGeoBBox(layer->getGeoExtent().boundingRect());

    // By default pixel extent is (0,0,w,h) = imageDataProvider.pixelExent
    // other use user specified userPixelExtent
    if (userPixelExtent.isEmpty())
        layer->setPixelExtent(provider->getPixelExtent());
    else
        layer->setPixelExtent(userPixelExtent);

    layer->setProjectionRef(provider->fetchProjectionRef());
    layer->setGeoTransform(provider->fetchGeoTransform());
    // !!! NEED TO ADD METADATA
//    layer->setMetadata(imageDataProvider->fetchMetadata());
    return layer;
}

//******************************************************************************

Core::GeoImageItem * GeoImageViewer::getGeoImageItem(Core::BaseLayer * layer)
{
    Core::GeoImageLayer * image = qobject_cast<Core::GeoImageLayer*>(layer);
    if (!image)
    {
        return 0;
    }
    // static cast is ensured with non null conversion of BaseLayer into GeoImageLayer
    return static_cast<Core::GeoImageItem*>(_layerItemMap.value(image, 0));
}

//******************************************************************************

const Core::ImageDataProvider * GeoImageViewer::getDataProvider(Core::BaseLayer * layer)
{
    Core::GeoImageItem* item = getGeoImageItem(layer);
    if (!item)
    {
        return 0;
    }
    return item->getConstDataProvider();
}

//******************************************************************************

void GeoImageViewer::enableOptions(bool v)
{
    setZoomEnabled(v);
    _settings.enableKeyNavigation=v;
    _settings.enableScroll=v;
    _enableTools = v;
}

//******************************************************************************

}
