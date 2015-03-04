#ifndef GEOIMAGEVIEWER_H
#define GEOIMAGEVIEWER_H

// Project
#include "Core/LibExport.h"
#include "Gui/ShapeViewer.h"

class QProgressDialog;
class QGraphicsItem;

namespace Core {
class ImageOpener;
class ImageWriter;
class ImageDataProvider;
class GeoImageItem;
}

namespace Tools {
class SelectionTool;
}

namespace Gui
{

class AbstractRendererView;

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
    virtual void onImageOpened(Core::ImageDataProvider *imageDataProvider);
    virtual void onImageWriteFinished(bool ok);
    virtual void onProgressCanceled();
//    virtual void onItemCreated(QGraphicsItem*);
    virtual void onCopyData(const QRectF & selection);
    virtual void onRenderConfigurationChanged();
    virtual void onBaseLayerSelected(Core::BaseLayer*);
    virtual void onSaveBaseLayer(Core::BaseLayer*);
    virtual void onFilterTriggered();

protected:

    virtual bool onSceneDragAndDrop(const QList<QUrl> & urls);

    Core::GeoImageItem * getGeoImageItem(Core::BaseLayer * layer);
    void enableOptions(bool v);

    Core::ImageOpener * _imageOpener;
    Core::ImageWriter * _imageWriter;

    AbstractRendererView * _rendererView;

    Tools::SelectionTool * _selection;



};

//******************************************************************************

}

#endif // GEOIMAGEVIEWER_H
