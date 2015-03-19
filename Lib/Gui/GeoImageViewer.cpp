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
#include "LayersView.h"
#include "DefaultFilterDialog.h"
#include "Core/Global.h"
#include "Core/ImageOpener.h"
#include "Core/ImageWriter.h"
#include "Core/GeoImageItem.h"
#include "Core/GeoImageLayer.h"
#include "Core/ImageDataProvider.h"
#include "Core/FloatingDataProvider.h"
#include "Core/HistogramImageRenderer.h"
#include "Core/LayerUtils.h"
#include "Tools/SelectionTool.h"
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

    // Connect with SelectionTool :
    _selection = new Tools::SelectionTool();
    _toolsManager->insertTool(_selection);
    connect(_selection, SIGNAL(copyToNewLayer(QRectF)), this, SLOT(onCopyData(QRectF)));


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

void GeoImageViewer::onImageOpened(Core::ImageDataProvider *imageDataProvider)
{

    enableOptions(true);



    if (!imageDataProvider)
    {
        SD_ERR("Application failed to read the image data");
        _progressDialog->close();
        return;
    }

    // Prepare the Scene/View:
        // remove initial text and set background to black
    _scene.removeItem(_initialTextItem);
    delete _initialTextItem;
        // set visible whole image + boundaries:
    int w =imageDataProvider->getWidth();
    int h =imageDataProvider->getHeight();
    _scene.setSceneRect(
                QRectF(imageDataProvider->getPixelExtent())
                .adjusted(-0.25*w, -0.25*h, 0.25*w, 0.25*h)
                );
    _view.setBackgroundBrush(QBrush(Qt::black));


    // Create Geo Image Item from data provider
//        // create a renderer
//    Core::HistogramImageRenderer * renderer =  new Core::HistogramImageRenderer();
//    renderer->setupConfiguration(imageDataProvider);

//        // create a GeoImageItem, GraphicsScene is responsible to delete it
//    Core::GeoImageItem * item = new Core::GeoImageItem();
//    item->setRenderer(renderer);
//    item->setDataProvider(imageDataProvider);
//    _scene.addItem(item);
//    connect(this, SIGNAL(viewportChanged(int,QRectF)),
//            item, SLOT(updateItem(int, QRectF)));

    Core::GeoImageItem * item = createGeoImageItem(imageDataProvider);

    // Add to layers storage
    Core::GeoImageLayer * layer = new Core::GeoImageLayer(this);
    layer->setType("Image");
    layer->setImageName(imageDataProvider->getImageName());
    layer->setNbBands(imageDataProvider->getInputNbBands());
    layer->setDepthInBytes(imageDataProvider->getInputDepthInBytes());
    layer->setIsComplex(imageDataProvider->inputIsComplex());

    layer->setGeoExtent(imageDataProvider->fetchGeoExtent());
    layer->setGeoBBox(layer->getGeoExtent().boundingRect());
    // pixel extent is (0,0,w,h) = imageDataProvider.pixelExent
    layer->setPixelExtent(imageDataProvider->getPixelExtent());
    layer->setProjectionRef(imageDataProvider->fetchProjectionRef());

    addLayer(layer, item);



    // configure renderer view :
    if (_rendererView)
    {
        _rendererView->clear();
        _rendererView->setup(item->getRenderer(), item->getConstDataProvider());
    }

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

void GeoImageViewer::onRenderConfigurationChanged()
{

    // IDEA : RendererView -> Renderer => Renderer.parent -> GeoImageItem
    // -> GeoImageItem.clearCache
    // -> apply new renderer conf
    // -> GeoImageItem.updateItem();
    const Core::ImageRenderer * r = _rendererView->getRenderer();
    if (!r)
    {
        SD_TRACE("GeoImageViewer::onRenderConfigurationChanged : Renderer is null");
        return;
    }

    Core::GeoImageItem * item = qobject_cast<Core::GeoImageItem*>(r->parent());
    if (!item)
    {
        SD_TRACE("GeoImageViewer::onRenderConfigurationChanged : GeoImageItem is null");
        return;
    }
    // clear all tiles
    item->clearCache();
    // apply new render conf
    _rendererView->applyNewRendererConfiguration();
    // load new tiles
    item->updateItem(_zoomLevel, getVisibleSceneRect());
}

//******************************************************************************

void GeoImageViewer::onBaseLayerSelected(Core::BaseLayer * layer)
{
    SD_TRACE("GeoImageViewer::onBaseLayerSelected");
    ShapeViewer::onBaseLayerSelected(layer);

    // setup renderer view if layer is geo image layer
    Core::GeoImageItem * imItem = getGeoImageItem(layer);
    if (!imItem)
        return;

    if (_rendererView)
    {
        _rendererView->clear();
        _rendererView->setup(imItem->getRenderer(),
                             imItem->getConstDataProvider());
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

    if (!_imageWriter->writeInBackground(filename, provider))
    {
        _progressDialog->close();
    }
}

//******************************************************************************

void GeoImageViewer::onFilterTriggered()
{
    filterGeoImageLayer(_layersView->getCurrentLayer());
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
        SD_TRACE("GeoImageViewer::onFilterTriggered : failed to get qaction");
        return;
    }

    Filters::AbstractFilter * f = qobject_cast<Filters::AbstractFilter*>(a->data().value<QObject*>());
    if (!f)
    {
        SD_TRACE("GeoImageViewer::onFilterTriggered : failed to get filter");
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

//void GeoImageViewer::onItemCreated(QGraphicsItem * item)
//{
//    SD_TRACE("GeoImageViewer::onItemCreated");
//    // create layer:
//    Core::GeoShapeLayer * layer = new Core::GeoShapeLayer(this);
//    // set geo info:
//    layer->setPixelExtent(item->boundingRect().toRect());
////    layer->setGeoExtent();
////    layer->setGeoBBox(layer->getGeoExtent().boundingRect());
//    layer->setType(_currentTool->getName());
//    addLayer(layer, item);
//}

//******************************************************************************
/*!
 * \brief GeoImageViewer::setRendererView method to setup a renderer view: DefaultRendererView or HistogramRendererView ...
 * \param rendererView
 */
void GeoImageViewer::setRendererView(AbstractRendererView *rendererView)
{
    _rendererView = rendererView;
    connect(_rendererView, SIGNAL(renderConfigurationChanged()), this, SLOT(onRenderConfigurationChanged()));
}

//******************************************************************************

void GeoImageViewer::onCopyData(const QRectF &selection)
{
    SD_TRACE("Copy data to new layer");

    Core::GeoImageLayer * iLayer = qobject_cast<Core::GeoImageLayer*>(_layersView->getCurrentLayer());
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
        // Create renderer:
//    Core::HistogramImageRenderer * renderer =  new Core::HistogramImageRenderer();
//    renderer->setupConfiguration(nProvider);

//        // Create geo image item :
//    Core::GeoImageItem * nItem = new Core::GeoImageItem();
//    nItem->setPos(selection.topLeft());
//    nItem->setDataProvider(nProvider);
//    nItem->setRenderer(renderer);
//    _scene.addItem(nItem);
//    connect(this, SIGNAL(viewportChanged(int,QRectF)),
//            nItem, SLOT(updateItem(int, QRectF)));

    Core::GeoImageItem * nItem = createGeoImageItem(nProvider, selection.topLeft());

    // Create new layer :
    Core::GeoImageLayer * nLayer = new Core::GeoImageLayer(this);
    nLayer->setType("Floating Image");
    nLayer->setImageName(nProvider->getImageName());

    nLayer->setNbBands(iLayer->getNbBands());
    nLayer->setDepthInBytes(iLayer->getDepthInBytes());
    nLayer->setIsComplex(iLayer->isComplex());

    nLayer->setGeoExtent(nProvider->fetchGeoExtent());
    nLayer->setGeoBBox(nLayer->getGeoExtent().boundingRect());
    // pixel extent is intersection
    nLayer->setPixelExtent(intersection);
    nLayer->setProjectionRef(iLayer->getProjectionRef());


    addLayer(nLayer, nItem);

    // display item in Scene:
    nItem->updateItem(_zoomLevel, getVisibleSceneRect());


}

//******************************************************************************

Core::GeoImageItem * GeoImageViewer::createGeoImageItem(Core::ImageDataProvider * provider, const QPointF & pos)
{
    // Create renderer:
    Core::HistogramImageRenderer * renderer =  new Core::HistogramImageRenderer();
    renderer->setupConfiguration(provider);

    // Create geo image item :
    Core::GeoImageItem * out = new Core::GeoImageItem();
    out->setPos(pos);
    out->setDataProvider(provider);
    out->setRenderer(renderer);
    _scene.addItem(out);
    connect(this, SIGNAL(viewportChanged(int,QRectF)),
            out, SLOT(updateItem(int, QRectF)));

    return out;
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

void GeoImageViewer::enableOptions(bool v)
{
    setZoomEnabled(v);
    _settings.enableKeyNavigation=v;
    _settings.enableScroll=v;
    _enableTools = v;
}

//******************************************************************************

}
