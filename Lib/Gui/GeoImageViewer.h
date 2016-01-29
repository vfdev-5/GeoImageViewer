#ifndef GEOIMAGEVIEWER_H
#define GEOIMAGEVIEWER_H

// Project
#include "Core/LibExport.h"
#include "Gui/ShapeViewer.h"

class QProgressDialog;
class QGraphicsItem;
class QCloseEvent;

namespace Core {
class ImageOpener;
class ImageWriter;
class ImageDataProvider;
class GeoImageItem;
class GeoImageLayer;
class GeoShapeLayer;
class DrawingsItem;
}

namespace Tools {
class SelectionTool;
}

namespace Gui
{

class AbstractRendererView;
class FilteringView;

//******************************************************************************

class GIV_DLL_EXPORT GeoImageViewer : public ShapeViewer
{
    Q_OBJECT

public:
    explicit GeoImageViewer(QWidget *parent = 0);
    ~GeoImageViewer();

    void loadImage(const QUrl & url);
    virtual void clear();
    void setRendererView(AbstractRendererView * rendererView );

protected slots:
    virtual void onProgressCanceled();

    virtual void onImageOpened(Core::ImageDataProvider *imageDataProvider);
    virtual void onCopyData(const QRectF & selection);
    virtual void onBaseLayerSelected(Core::BaseLayer*);
    virtual void onBaseLayerDestroyed(QObject *);
    virtual void onCreateBaseLayer();

    virtual void onSaveBaseLayer(Core::BaseLayer*);
    virtual void onImageWriteFinished(bool ok);

    virtual void onToolChanged(const QString &);

    virtual void onFilterTriggered();
    virtual void onFilteringFinished(Core::ImageDataProvider *);

    void onDrawingFinalized(const QString&, Core::DrawingsItem*);

protected:

    void writeGeoImageLayer(Core::GeoImageLayer *layer);
    void writeGeoShapeLayer(Core::GeoShapeLayer *layer);

    const Core::ImageDataProvider *getDataProvider(const Core::BaseLayer * layer) const;
    Core::GeoImageItem * createGeoImageItem(Core::ImageDataProvider *, const QPointF &pos=QPointF());
    Core::GeoImageLayer * createGeoImageLayer(const QString & type, Core::GeoImageItem *item, Core::ImageDataProvider * provider, const QRect &userPixelExtent = QRect());
    Core::GeoImageLayer * createScribble(const QString & name, Core::DrawingsItem * item, const Core::ImageDataProvider *provider = 0);

    void prepareSceneAndView(int w, int h);

    bool configureTool(Tools::AbstractTool * tool, Core::BaseLayer * layer);

    void initFilterTools();

    virtual bool onSceneDragAndDrop(const QList<QUrl> & urls);
    void enableOptions(bool v);

    virtual QVector<double> getPixelValues(const QPoint &point, bool * isComplex = 0) const;
    virtual QPointF computePointOnItem(const QPointF &scenePos);
    virtual QPolygonF computeGeoExtentFromLayer(const QPolygonF & inputShape, const Core::GeoShapeLayer * backgroundLayer);

    Core::ImageOpener * _imageOpener;
    Core::ImageWriter * _imageWriter;

    AbstractRendererView * _rendererView;

    FilteringView * _filteringView;

    Core::GeoImageLayer * _processedLayer;

};

//******************************************************************************

}

#endif // GEOIMAGEVIEWER_H
