
// Qt
#include <qmath.h>
#include <QWheelEvent>
#include <QGraphicsSimpleTextItem>
#include <QMimeData>
#include <QGraphicsSceneDragDropEvent>
#include <QProgressDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QUrl>
#include <QFileInfo>
#include <QScrollBar>

// Project
#include "GIViewer.h"
#include "AbstractRendererView.h"
#include "AbstractToolsView.h"
#include "SubdatasetDialog.h"
#include "Core/Global.h"
#include "Core/ImageManager.h"
#include "Core/Image.h"
#include "Core/ImageLayer.h"
#include "Core/LayerLoader.h"
#include "Core/HistogramLayerRenderer.h"
#include "Core/LayerTools.h"
#include "Tools/ToolsManager.h"
#include "Tools/AbstractTool.h"


namespace Gui
{

QMouseEvent storeMouseEvent(QMouseEvent* event)
{
    return QMouseEvent(QEvent::MouseMove,
                                  event->localPos(),
                                  event->windowPos(),
                                  event->screenPos(),
                                  event->button(),
                                  event->buttons(),
                                  event->modifiers());
}


//******************************************************************************
/*!
* \class GIViewer
*
* \brief This is a widget that displays image.
*   - Its method loadImage() with image path loads image.
*   - Method clear() restarts the viewer.
*   - Imagery is then represented by classes Image and ImageLayer that are stored in the map of
*     the singleton ImageManager
*/
//******************************************************************************

GIViewer::GIViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui_GIViewer),
    _zoomLevel(0),
    _zoomMinLevel(0),
    _zoomMaxLevel(6),
    _rendererView(0),
    _currentTool(0),
    _handScrolling(false),
    _lastMouseEvent(QEvent::None, QPointF(), QPointF(), QPointF(), Qt::NoButton, 0, 0)
{

    ui->setupUi(this);

    // setup scene:
    _scene.setSceneRect(0.0, 0.0, 1.0, 1.0);
    _scene.installEventFilter(this);

    // init Renderer:
    _renderer = new Core::HistogramLayerRenderer();
//    _renderer = new Core::LayerRenderer();

    // init Loader
    _loader = new Core::LayerLoader(this);
    _loader->setScene(&_scene);
    _loader->setRenderer(_renderer);

    // Init scene and loader
    clear();

    // setup view
    ui->_view->setScene(&_scene);
    // hardware acceleration
    ui->_view->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    ui->_view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//        ui->_view->setDragMode(QGraphicsView::ScrollHandDrag);

    ui->_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->_view->setAcceptDrops(true);
    ui->_view->installEventFilter(this);
    ui->_view->viewport()->installEventFilter(this);


    // setup progress dialog:
    _progressDialog = new QProgressDialog(this);
    _progressDialog->setWindowModality(Qt::WindowModal);
    connect(_progressDialog, SIGNAL(canceled()), this, SLOT(onImageLoadCanceled()));

    // init ImageManager
    _imageManager = Core::ImageManager::get();
    connect(_imageManager, SIGNAL(loadFinished(Core::Image*)), this, SLOT(onImageLoaded(Core::Image*)));
    connect(_imageManager, SIGNAL(loadProgressValueChanged(int)), this, SLOT(onImageLoadProgressValueChanged(int)));


    // init ToolsManager
    _toolsManager = Tools::ToolsManager::get();

}

//******************************************************************************

GIViewer::~GIViewer()
{
    delete ui;
    Core::ImageManager::destroy();
    Tools::ToolsManager::destroy();

}

//******************************************************************************

void GIViewer::clear()
{
    _loader->clear();
    _scene.clear();
    if (_rendererView)
    {
        _rendererView->clear();
    }

    _initialText = _scene.addSimpleText(tr("Drag and drop an image"));
    _initialText->setPen(QColor(167,167,167));
    _initialText->setScale(0.005 * qMax(_scene.sceneRect().width(), _scene.sceneRect().height()));
    double w = _initialText->boundingRect().width() * _initialText->scale();
    double h = _initialText->boundingRect().height() * _initialText->scale();
    double x = _scene.sceneRect().center().x() - w*0.5;
    double y = _scene.sceneRect().center().y() - h*0.5;
    _initialText->setPos(x, y);
    _initialText->setOpacity(0.3);


    ui->_view->setBackgroundBrush(QBrush(Qt::white));
    ui->_view->fitInView(_scene.sceneRect(), Qt::KeepAspectRatio);

}

//******************************************************************************

void GIViewer::showEvent(QShowEvent *e)
{
    Q_UNUSED(e)
    ui->_view->fitInView(_scene.sceneRect(), Qt::KeepAspectRatio);
}

//******************************************************************************

void GIViewer::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    if (_loader->hasLayer())
        centerOnAtZoom(_zoomLevel);
}

//******************************************************************************
/*
 * Event filter :
 * 1) Drag/Drop images when no images
 * 2) If has loaded image
 *  a) navigation on keys
 *  b) zoom on wheel
 *  c) reload tiles on scroll
 */
bool GIViewer::eventFilter(QObject * o, QEvent * e)
{

    if (_loader->hasLayer())
    {
        if (o == &_scene)
        {
            if (_currentTool && _currentTool->dispatch(e, &_scene))
            {
                e->accept();
                return true;
            }
        }
        else if (o == ui->_view)
        {
            if (navigationOnKeys(e))
            {
                e->accept();
                return true;
            }
            else if (scrollOnMouse(e))
            {
                e->accept();
                return true;
            }

        }
        else if (o == ui->_view->viewport())
        {
            if (zoomOnWheelEvent(e))
            {
                e->accept();
                return true;
            }
            else
            {
                // this is need to propagate event to QGraphicsView
                e->ignore();
            }
        }
    }
    else
    {
        if (dragAndDropEventOnScene(e))
            return true;
    }

    return QWidget::eventFilter(o, e);
}

//******************************************************************************

bool GIViewer::dragAndDropEventOnScene(QEvent *e)
{
    if (e->type() == QEvent::GraphicsSceneDragEnter ||
            e->type() == QEvent::GraphicsSceneDragMove)
    {
        QGraphicsSceneDragDropEvent *event = static_cast<QGraphicsSceneDragDropEvent*>(e);
        if (event->mimeData()->hasUrls())
        {
            event->acceptProposedAction();
            return true;
        }

    }
    else if (e->type() == QEvent::GraphicsSceneDrop)
    {
        QGraphicsSceneDragDropEvent *event = static_cast<QGraphicsSceneDragDropEvent*>(e);
        if (event->mimeData()->hasUrls())
        {
            foreach (QUrl url, event->mimeData()->urls())
            {
                SD_TRACE("Drop : " + url.toLocalFile());
                loadImage(url.toLocalFile());
                break;
            }

        }
        return true;
    }
    return false;
}

//******************************************************************************

bool GIViewer::navigationOnKeys(QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent * event = static_cast<QKeyEvent*>(e);
        QPointF sceneCenter = getVisibleSceneRect().center();
        double step = 15.0;
        if (event->key() == Qt::Key_Right)
        {
            sceneCenter += QPointF(step,0.0);
        }
        else if (event->key() == Qt::Key_Left)
        {
            sceneCenter -= QPointF(step,0.0);
        }
        else if (event->key() == Qt::Key_Up)
        {
            sceneCenter -= QPointF(0.0,step);
        }
        else if (event->key() == Qt::Key_Down)
        {
            sceneCenter += QPointF(0.0,step);
        }
        centerOnAtZoom(_zoomLevel, sceneCenter);
        event->accept();
        return true;
    }
    return false;
}

//******************************************************************************

/*!
 *  Method to emulate QGraphicsView::ScrollHandDrag option for both mouse buttons
 *  Therefore, tools can navigate using right mouse button
 */
bool GIViewer::scrollOnMouse(QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress)
    {
        _handScrolling = true;
        ui->_view->viewport()->setCursor(Qt::ClosedHandCursor);
        _lastMouseEvent = storeMouseEvent(static_cast<QMouseEvent*>(e));
        return true;
    }
    else if (e->type() == QEvent::MouseMove)
    {
        if (_handScrolling)
        {
            QMouseEvent * event = static_cast<QMouseEvent*>(e);
            QScrollBar *hBar = ui->_view->horizontalScrollBar();
            QScrollBar *vBar = ui->_view->verticalScrollBar();
            QPoint delta = event->pos() - _lastMouseEvent.pos();
            hBar->setValue(hBar->value() + (ui->_view->isRightToLeft() ? delta.x() : -delta.x()));
            vBar->setValue(vBar->value() - delta.y());

            _lastMouseEvent = storeMouseEvent(event);

            return true;
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        _handScrolling = false;
        ui->_view->viewport()->setCursor(_currentTool->getCursor());
        centerOnAtZoom(_zoomLevel);
        return true;
    }
    return false;
}

//******************************************************************************

bool GIViewer::zoomOnWheelEvent(QEvent * e)
{
    if (e->type() == QEvent::Wheel)
    {
        QWheelEvent * event = static_cast<QWheelEvent*>(e);
        // mouse has X units that covers 360 degrees. Zoom when 15 degrees is provided
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QPoint numDegrees = event->angleDelta() / 8;
#else
        QPoint numDegrees(0,event->delta() / 8);
#endif
        if (!numDegrees.isNull())
        {
            if (numDegrees.y() >= 15)
            { // Zoom in
                if (_zoomLevel+1<=_zoomMaxLevel)
                    centerOnAtZoom(_zoomLevel+1);
            }
            else if (numDegrees.y() <= -15)
            { // Zoom out
                if (_zoomLevel-1>=_zoomMinLevel)
                    centerOnAtZoom(_zoomLevel-1);
            }
            event->accept();
            return true;
        }
        return true;
    }
    return false;
}

//******************************************************************************

void GIViewer::setupPlugins(const QString &pluginsPath)
{
    _toolsManager->loadPlugins(pluginsPath);
}

//******************************************************************************

void GIViewer::loadImage(const QString &filepath)
{
    // reset previous state
    clear();


    // pre-loading phase :
    if (!QFileInfo(filepath).exists())
    {
        SD_ERR(tr("File %1 is not found").arg(filepath));
        return;
    }

    QString fileToOpen=filepath;
    // check if image has subsets
    QStringList subsetNames, subsetDesriptions;
    if (Core::isSubsetFile(fileToOpen, subsetNames, subsetDesriptions))
    { // Image has subsets
        if (subsetNames.size() > 1)
        { // Image has more than one subset -> ask user to choose

            SubdatasetDialog dialog(subsetDesriptions);

            if (!dialog.exec())
            {
                return;
            }
            else
            {
                int index = dialog.getSelectionIndex();
                if (index < 0 || index >= subsetNames.size())
                    return;
                fileToOpen = subsetNames[index];
            }
        }
        else
        {
            fileToOpen = subsetNames.first();
        }
    }

    _progressDialog->setLabelText("Load image ...");
    _progressDialog->setValue(0);
    _progressDialog->show();

    _imageManager->loadImageInBackground(fileToOpen, filepath);

}

//******************************************************************************

void GIViewer::onImageLoaded(Core::Image * image)
{
    _progressDialog->close();

    if (!image)
    {
//        SD_TRACE("GIViewer::onImageLoaded : Image is null");
        SD_ERR("Application failed to read the image data")
        return;
    }
    Core::ImageLayer * layer = _imageManager->getImageLayer(image);
    if (!layer)
    {
        SD_TRACE("GIViewer::onImageLoaded : Layer is null")
        return;
    }

    // setup default rendering configuration:
    _renderer->setupConfiguration(layer);

    // configure renderer view :
    if (_rendererView)
    {
        _rendererView->clear();
        _rendererView->setup(_renderer, layer);
    }

    _loader->setLayer(layer);
    _zoomMinLevel=_loader->computeZoomMinLevel();

    // remove initial text and set background to black
    _scene.removeItem(_initialText);
    _scene.setSceneRect(0.0,0.0,layer->getWidth(),layer->getHeight());

    ui->_view->setBackgroundBrush(QBrush(Qt::black));

    // zoom
    centerOnAtZoom(_zoomMinLevel, QPointF(layer->getWidth()*0.5,layer->getHeight()*0.5));

}

//******************************************************************************

void GIViewer::onImageLoadProgressValueChanged(int value)
{
    _progressDialog->setValue(value);
}

//******************************************************************************

void GIViewer::onImageLoadCanceled()
{
    _imageManager->onLoadCanceled();
}

//******************************************************************************

void GIViewer::onRenderConfigurationChanged()
{
    // clear all loaded tiles
    _loader->clearCache();
    // apply new render conf
    _rendererView->applyNewRendererConfiguration();
    // load new tiles
    centerOnAtZoom(_zoomLevel);
}

//******************************************************************************

void GIViewer::onToolChanged(QString toolName)
{
    SD_TRACE("onToolChanged : " + toolName);

    // cancel all pending drawings
//    _currentTool

    // change to the new one
    _currentTool = _toolsManager->getTool(toolName);
    ui->_view->viewport()->setCursor(_currentTool->getCursor());
}

//******************************************************************************

void GIViewer::centerOnAtZoom(int zoomLevel, const QPointF &scenePoint)
{
    if (zoomLevel != _zoomLevel)
    {
        _zoomLevel=zoomLevel;
        double scale = qPow(2.0, zoomLevel);
        ui->_view->setTransform(QTransform::fromScale(scale,scale));
    }

    if (!scenePoint.isNull())
    {
        ui->_view->centerOn(scenePoint);
    }

//    viewportInfo();

    // start tile loading:
    _loader->updateLayer(zoomLevel, getVisibleSceneRect().toRect());

}

//******************************************************************************

QRectF GIViewer::getVisibleSceneRect()
{
    QRect wr = ui->_view->viewport()->rect();
    return ui->_view->mapToScene(wr).boundingRect();
}

//******************************************************************************
/*!
 * \brief GIViewer::setRendererView method to setup a renderer view: LayerRendererView or HistogramRendererView ...
 * \param rendererView
 */
void GIViewer::setRendererView(AbstractRendererView *rendererView)
{
    _rendererView = rendererView;
    connect(_rendererView, SIGNAL(renderConfigurationChanged()), this, SLOT(onRenderConfigurationChanged()));
}

//******************************************************************************

void GIViewer::setToolsView(AbstractToolsView *toolsView)
{
    _toolsView = toolsView;
    connect(_toolsView, SIGNAL(toolChanged(QString)), this, SLOT(onToolChanged(QString)));

    QList<Tools::AbstractTool*> list = _toolsManager->getTools();
    foreach (Tools::AbstractTool* t, list)
    {
        _toolsView->addTool(t);
    }
    _toolsView->setCurrentTool(list.first()->getName());

}

//******************************************************************************

void GIViewer::viewportInfo()
{
    // display visible scene zone
    QRect wr = ui->_view->viewport()->rect();
    SD_TRACE(QString("view rect : (%1, %2) | %3, %4")
             .arg(wr.x())
             .arg(wr.y())
             .arg(wr.width())
             .arg(wr.height()));

    QRectF r = ui->_view->mapToScene(wr).boundingRect();
    SD_TRACE(QString("visible scene rect : (%1, %2) | %3, %4")
             .arg(r.x())
             .arg(r.y())
             .arg(r.width())
             .arg(r.height()));

    double sx = ui->_view->matrix().m11();
    double sy = ui->_view->matrix().m22();
    SD_TRACE(QString("view scale : %1, %2").arg(sx).arg(sy));

}

//******************************************************************************

}
