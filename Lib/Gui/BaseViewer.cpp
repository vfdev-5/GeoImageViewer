
// Qt
#include <qmath.h>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSimpleTextItem>
#include <QProgressDialog>
#include <QGLWidget>
#include <QFileInfo>
#include <QScrollBar>
#include <QAction>
#include <QMenu>
#include <QLabel>

// Project
#include "BaseViewer.h"
#include "Core/Global.h"

//#define SHOW_VIEWPORT_INFO

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
* \class BaseViewer
*
* \brief This class provides a simple base realization of GraphicsScene/View system.
* The idea to provide a base class of scene navigation : zoom/scroll/arrow keys.
*
*
*   - Method clear() restarts the viewer.
*/
//******************************************************************************

BaseViewer::BaseViewer(const QString &initialText, QWidget *parent) :
    QWidget(parent),
    _zoomLevel(0),
    _zoomMinLevel(-5),
    _zoomMaxLevel(5),
    _initialText(initialText),
    _pointInfo(new QLabel()),
    _handScrolling(false),
    _lastMouseEvent(QEvent::None, QPointF(), QPointF(), QPointF(), Qt::NoButton, 0, 0)
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addWidget(&_view);
    layout->addWidget(_pointInfo);
    _pointInfo->setVisible(false);

    _scene.installEventFilter(this);

    // Init scene
    clear();

    // setup view
    _view.setScene(&_scene);
    _view.installEventFilter(this);
    // hardware acceleration
    _view.setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    _view.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    _view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    _view.setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    _view.setAcceptDrops(true);
    _view.viewport()->installEventFilter(this);

    // setup menu
    setupViewContextMenu();

    // setup progress dialog:
    _progressDialog = new QProgressDialog(this);
    _progressDialog->setWindowModality(Qt::WindowModal);
    _progressDialog->setAutoClose(true);
    _progressDialog->setAutoReset(true);
    connect(_progressDialog, SIGNAL(canceled()), this, SLOT(onProgressCanceled()));

}

//******************************************************************************

void BaseViewer::clear()
{
    SD_TRACE("BaseViewer::clear");

    _scene.clear();

    // setup scene:
    _scene.setSceneRect(0.0, 0.0, 200.0, 200.0);

    if (!_initialText.isEmpty())
    {
        _initialTextItem = _scene.addSimpleText(_initialText);
        _initialTextItem->setPen(QColor(167,167,167));
        _initialTextItem->setScale(0.015 * qMax(_scene.sceneRect().width(), _scene.sceneRect().height()));
        double w = _initialTextItem->boundingRect().width() * _initialTextItem->scale();
        double h = _initialTextItem->boundingRect().height() * _initialTextItem->scale();
        double x = _scene.sceneRect().center().x() - w*0.5;
        double y = _scene.sceneRect().center().y() - h*0.5;
        _initialTextItem->setPos(x, y);
        _initialTextItem->setOpacity(0.3);
    }

    _view.setBackgroundBrush(QBrush(Qt::white));

    // reset zoom level
    _zoomLevel = 0;
    double scale = qPow(2.0, _zoomLevel);
    _view.setTransform(QTransform::fromScale(scale,scale));

}

//******************************************************************************

void BaseViewer::showEvent(QShowEvent *e)
{
    SD_TRACE("BaseViewer::showEvent");
    Q_UNUSED(e)
    centerOnAtZoom(_zoomLevel, getVisibleSceneRect().center());
#ifdef _DEBUG
    viewportInfo();
#endif

}

//******************************************************************************

void BaseViewer::resizeEvent(QResizeEvent *e)
{
    SD_TRACE("BaseViewer::resizeEvent");
    Q_UNUSED(e)
#ifdef _DEBUG
    viewportInfo();
#endif
    centerOnAtZoom(_zoomLevel, getVisibleSceneRect().center());
}

//******************************************************************************

void BaseViewer::onProgressValueChanged(int value)
{
    _progressDialog->setValue(value);
}

//******************************************************************************

void BaseViewer::onZoomActionTriggered()
{
    if (sender() == _zoomIn)
    {
        if (_zoomLevel+1<=_zoomMaxLevel)
        {
            centerOnAtZoom(_zoomLevel+1,
                           _view.mapToScene(
                               _view.mapFromGlobal(_menu.pos())
                               ));
        }
    }
    else if (sender() == _zoomOut)
    {
        if (_zoomLevel-1>=_zoomMinLevel)
        {
            centerOnAtZoom(_zoomLevel-1,
                           _view.mapToScene(
                               _view.mapFromGlobal(_menu.pos())
                               ));
        }
    }
}

//******************************************************************************

void BaseViewer::onContextMenuRequested(QPoint p)
{
    SD_TRACE("onContextMenuRequested : " + QString::number(_menu.actions().size()));
    _menu.popup(_view.mapToGlobal(p));
}

//******************************************************************************
/*
 * Event filter :
 * 1) Drag/Drop images when no images
 * 2) navigation on keys
 * 3) zoom on wheel
 * 4) scroll
 */
bool BaseViewer::eventFilter(QObject * o, QEvent * e)
{

    if (o == &_scene &&
            _settings.enableSceneDragAndDropEvent &&
            dragAndDropEventOnScene(e))
    {
        return true;
    }
    else if (o == &_view)
    {
        if (_settings.enableKeyNavigation &&
                navigationOnKeys(e))
        {
            e->accept();
            return true;
        }
        else if (_settings.enableScroll &&
                 scrollOnMouse(e))
        {
            e->accept();
            return true;
        }
    }

    if (o == _view.viewport())
    {
        // zoom on wheel should in viewport than graphicsview due to scroll wheel action in graphicsview
        if (_settings.enableZoom &&
                zoomOnWheelEvent(e))
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

    return QWidget::eventFilter(o, e);
}

//******************************************************************************

bool BaseViewer::dragAndDropEventOnScene(QEvent *e)
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
            return onSceneDragAndDrop(event->mimeData()->urls());
        }
    }
    return false;
}

//******************************************************************************

bool BaseViewer::navigationOnKeys(QEvent *e)
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
 *  Therefore, user can navigate using right mouse button
 */
bool BaseViewer::scrollOnMouse(QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress)
    {
        _handScrolling = true;
        _view.viewport()->setCursor(Qt::ClosedHandCursor);
        _lastMouseEvent = storeMouseEvent(static_cast<QMouseEvent*>(e));
        return true;
    }
    else if (e->type() == QEvent::MouseMove)
    {
        if (_handScrolling)
        {
            QMouseEvent * event = static_cast<QMouseEvent*>(e);
            QScrollBar *hBar = _view.horizontalScrollBar();
            QScrollBar *vBar = _view.verticalScrollBar();
            QPoint delta = event->pos() - _lastMouseEvent.pos();
            hBar->setValue(hBar->value() + (_view.isRightToLeft() ? delta.x() : -delta.x()));
            vBar->setValue(vBar->value() - delta.y());
            _lastMouseEvent = storeMouseEvent(event);
            return true;
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        _handScrolling = false;
        _view.viewport()->setCursor(_cursor);
        centerOnAtZoom(_zoomLevel);
        return true;
    }
    return false;
}

//******************************************************************************

bool BaseViewer::zoomOnWheelEvent(QEvent * e)
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

void BaseViewer::centerOnAtZoom(int zoomLevel, const QPointF &scenePoint)
{
    if (zoomLevel != _zoomLevel)
    {
        _zoomLevel=zoomLevel;
        double scale = qPow(2.0, zoomLevel);
        _view.setTransform(QTransform::fromScale(scale,scale));
    }

    if (!scenePoint.isNull())
    {
        _view.centerOn(scenePoint);
    }

#ifdef _DEBUG
    viewportInfo();
#endif
    emit viewportChanged(zoomLevel, getVisibleSceneRect());
}

//******************************************************************************

void BaseViewer::setupViewContextMenu()
{
    _view.setContextMenuPolicy(Qt::CustomContextMenu);
    // Zoom in :
    _zoomIn = new QAction(tr("Zoom in"), &_view);
    _zoomIn->setEnabled(_settings.enableZoom);
    connect(_zoomIn, SIGNAL(triggered()), this, SLOT(onZoomActionTriggered()));
    _view.addAction(_zoomIn);

    // Zoom out :
    _zoomOut = new QAction(tr("Zoom out"), &_view);
    _zoomOut->setEnabled(_settings.enableZoom);
    connect(_zoomOut, SIGNAL(triggered()), this, SLOT(onZoomActionTriggered()));
    _view.addAction(_zoomOut);

    // menu
    _menu.addActions(_view.actions());
    connect(&_view, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onContextMenuRequested(QPoint)));
}

//******************************************************************************

void BaseViewer::setZoomEnabled(bool enabled)
{
    _settings.enableZoom = enabled;
    _zoomIn->setEnabled(_settings.enableZoom);
    _zoomOut->setEnabled(_settings.enableZoom);
}

//******************************************************************************

QRectF BaseViewer::getVisibleSceneRect()
{
    QRect wr = _view.viewport()->rect();
    return _view.mapToScene(wr).boundingRect();
}

//******************************************************************************

void BaseViewer::viewportInfo()
{
#ifdef SHOW_VIEWPORT_INFO
    // display visible scene zone
    QRect wr = _view.viewport()->rect();
    SD_TRACE(QString("view rect : (%1, %2) | %3, %4")
             .arg(wr.x())
             .arg(wr.y())
             .arg(wr.width())
             .arg(wr.height()));

    QRectF r = _view.mapToScene(wr).boundingRect();
    SD_TRACE(QString("visible scene rect : (%1, %2) | %3, %4")
             .arg(r.x())
             .arg(r.y())
             .arg(r.width())
             .arg(r.height()));

    double sx = _view.matrix().m11();
    double sy = _view.matrix().m22();
    SD_TRACE(QString("view scale : %1, %2").arg(sx).arg(sy));
#endif

}

//******************************************************************************

void BaseViewer::showPointInfo(bool v)
{
    _pointInfo->setVisible(v);
}

//******************************************************************************

void BaseViewer::setCurrentCursor(const QCursor & c)
{
    _view.viewport()->setCursor(c);
    _cursor = c;
}

//******************************************************************************

}
