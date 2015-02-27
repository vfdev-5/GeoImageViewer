#include "GeoImageViewer.h"

// Qt
#include <qmath.h>
#include <QProgressDialog>
#include <QFileInfo>

// Project
#include "GeoImageViewer.h"
#include "AbstractRendererView.h"
#include "SubdatasetDialog.h"
#include "LayersView.h"
#include "Core/Global.h"
#include "Core/ImageOpener.h"
#include "Core/GeoImageItem.h"
#include "Core/GeoImageLayer.h"
#include "Core/ImageDataProvider.h"
#include "Core/FloatingDataProvider.h"
#include "Core/HistogramImageRenderer.h"
#include "Core/LayerUtils.h"
#include "Tools/SelectionTool.h"
#include "Tools/ToolsManager.h"


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
    _imageOpener(new Core::ImageOpener(this))
{

    // Init scene and loader
    clear();

    connect(_progressDialog, SIGNAL(canceled()), this, SLOT(onImageOpenCanceled()));
    connect(_imageOpener, SIGNAL(imageOpened(Core::ImageDataProvider*)), this, SLOT(onImageOpened(Core::ImageDataProvider *)));
    connect(_imageOpener, SIGNAL(openProgressValueChanged(int)), this, SLOT(onImageOpenProgressValueChanged(int)));

    setMinimumSize(QSize(450, 450));

    // Connect with SelectionTool :
    _selection = new Tools::SelectionTool();
    _toolsManager->insertTool(_selection);
    connect(_selection, SIGNAL(copyToNewLayer(QRectF)), this, SLOT(onCopyData(QRectF)));

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

    _progressDialog->close();

    if (!imageDataProvider)
    {
        SD_ERR("Application failed to read the image data");
        return;
    }

    // create a renderer
    Core::HistogramImageRenderer * renderer =  new Core::HistogramImageRenderer();
    renderer->setupConfiguration(imageDataProvider);

    // configure renderer view :
    if (_rendererView)
    {
        _rendererView->clear();
        _rendererView->setup(renderer, imageDataProvider);
    }

    // remove initial text and set background to black
    _scene.removeItem(_initialTextItem);
    delete _initialTextItem;

    // set visible whole image + boundaries:
//    _scene.setSceneRect(QRectF(imageDataProvider->getPixelExtent()));
    int w =imageDataProvider->getWidth();
    int h =imageDataProvider->getHeight();
    _scene.setSceneRect(
                QRectF(imageDataProvider->getPixelExtent())
                .adjusted(-0.25*w, -0.25*h, 0.25*w, 0.25*h)
                );

    _view.setBackgroundBrush(QBrush(Qt::black));

    // create a GeoImageItem, GraphicsScene is responsible to delete it
    Core::GeoImageItem * item = new Core::GeoImageItem();

    // TEST :
//    _scene.addRect(
//                imageDataProvider->getPixelExtent(),
//                QPen(Qt::black, 0),
//                QColor(250,250,250,50)
//                );
//    _scene.addRect(
//                QRectF(0.0,0.0,50.0,30.0),
//                QPen(Qt::black, 0),
//                QColor(250,0,0,100)
//                );
//    item->setPos(50.0, 30.0);
    // TEST :

    connect(this, SIGNAL(viewportChanged(int,QRectF)),
            item, SLOT(updateItem(int, QRectF)));



    item->setRenderer(renderer);
    item->setDataProvider(imageDataProvider);
    _scene.addItem(item);
    _zoomMinLevel = item->getZoomMinLevel();

    centerOnAtZoom(_zoomMinLevel, _scene.sceneRect().center());

    // Add to layers storage
    Core::GeoImageLayer * layer = new Core::GeoImageLayer(this);
    layer->setType("Image");
//    layer->setImageName(imageDataProvider->);
    layer->setNbBands(imageDataProvider->getInputNbBands());
    layer->setDepthInBytes(imageDataProvider->getInputDepthInBytes());
//    layer->setGeoBBox();
//    layer->setGeoExtent();
//    layer->setPixelExtent(imageDataProvider->getPixelExtent());
//    layer->setProjectionRef();

    addLayer(layer, item);
}

//******************************************************************************

void GeoImageViewer::onImageOpenProgressValueChanged(int value)
{
    _progressDialog->setValue(value);
}

//******************************************************************************

void GeoImageViewer::onImageOpenCanceled()
{
    _imageOpener->cancel();
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

//void GeoImageViewer::onBaseLayerDestroyed(QObject *object)
//{
//    ShapeViewer::onBaseLayerDestroyed(object);
//}

//******************************************************************************
/*!
 * \brief GeoImageViewer::setRendererView method to setup a renderer view: LayerRendererView or HistogramRendererView ...
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

    Core::GeoImageItem * item = getGeoImageItem(_layersView->getCurrentLayer());
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
    Core::FloatingDataProvider * nProvider = Core::FloatingDataProvider::createDataProvider(provider, pixelExtent);

    if (!nProvider)
    {
        SD_TRACE("onCopyData : new data provider is null");
        return;
    }

    // Create renderer:
    Core::HistogramImageRenderer * renderer =  new Core::HistogramImageRenderer();
//    Core::ImageRenderer * renderer =  new Core::ImageRenderer();
    renderer->setupConfiguration(nProvider);

    // Create geo image item :
    Core::GeoImageItem * nItem = new Core::GeoImageItem();
    nItem->setPos(selection.topLeft());
    nItem->setDataProvider(nProvider);
    nItem->setRenderer(renderer);
    _scene.addItem(nItem);

    nItem->updateItem(_zoomLevel, getVisibleSceneRect());


    connect(this, SIGNAL(viewportChanged(int,QRectF)),
            item, SLOT(updateItem(int, QRectF)));



    // Create new layer :
    Core::GeoImageLayer * nLayer = new Core::GeoImageLayer(this);
    nLayer->setType("Floating Image");

    addLayer(nLayer, nItem);

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
