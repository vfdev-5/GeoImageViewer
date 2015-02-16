#ifndef GIVIEWER_H
#define GIVIEWER_H

// Qt
#include <QWidget>
#include <QGraphicsScene>
#include <QShowEvent>
#include <QMouseEvent>
#include <QPaintEvent>


// Project
#include "Core/LibExport.h"
#include "ui_GIViewer.h"

class QProgressDialog;

namespace Core {
class Image;
class ImageManager;
class LayerLoader;
class LayerRenderer;
}

namespace Tools {
class ToolsManager;
class AbstractTool;
}

namespace Gui
{

class AbstractRendererView;
class AbstractToolsView;

//******************************************************************************


class GIV_DLL_EXPORT GIViewer : public QWidget
{
    Q_OBJECT

public:
    explicit GIViewer(QWidget *parent = 0);
    ~GIViewer();

    void setupPlugins(const QString & pluginsPath);
    void loadImage(const QString & filepath);
    void clear();

    void setRendererView(AbstractRendererView * rendererView );
    void setToolsView(AbstractToolsView * toolsView);

protected:
    virtual void showEvent(QShowEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    void centerOnAtZoom(int zoomLevel, const QPointF & scenePoint = QPointF());

    QRectF getVisibleSceneRect();

    void viewportInfo();


protected slots:
    void onImageLoaded(Core::Image *);
    void onImageLoadCanceled();
    void onImageLoadProgressValueChanged(int);

    void onRenderConfigurationChanged();
    void onToolChanged(QString);


private:

    bool dragAndDropEventOnScene(QEvent * e);
    bool navigationOnKeys(QEvent * e);
    bool zoomOnWheelEvent(QEvent * e);


    QMouseEvent _lastMouseEvent;
    bool _handScrolling;
    bool scrollOnMouse(QEvent * e);
    bool scrollOnMouse2(QEvent * e);


    Ui_GIViewer *ui;

    QGraphicsScene _scene;

    QGraphicsSimpleTextItem * _initialText;

    QProgressDialog * _progressDialog;
    Core::ImageManager * _imageManager;
    Core::LayerLoader * _loader;
    Core::LayerRenderer * _renderer;
    Tools::ToolsManager * _toolsManager;
    Tools::AbstractTool * _currentTool;


    AbstractRendererView * _rendererView;
    AbstractToolsView * _toolsView;

    int _zoomLevel;
    int _zoomMaxLevel;
    int _zoomMinLevel;
    QPointF _viewpoint;


};

//******************************************************************************

}

#endif // GIVIEWER_H
