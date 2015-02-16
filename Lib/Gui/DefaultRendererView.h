#ifndef DEFAULTRENDERERVIEW_H
#define DEFAULTRENDERERVIEW_H


// Qt
#include <QWidget>

// Project
#include "Core/LibExport.h"
#include "Core/LayerRenderer.h"
#include "ui_DefaultRendererView.h"
#include "AbstractRendererView.h"


namespace Core {
class ImageLayer;
}

namespace Gui
{

//******************************************************************************

class GIV_DLL_EXPORT DefaultRendererView : public AbstractRendererView
{
    Q_OBJECT
public:
    explicit DefaultRendererView(QWidget *parent = 0);
    virtual ~DefaultRendererView();
    virtual void clear();
    virtual void setup(Core::LayerRenderer * renderer, const Core::ImageLayer * layer);
    virtual void applyNewRendererConfiguration();

protected slots:
    void on__band_activated(int);
    void on__red_activated(int);
    void on__green_activated(int);
    void on__blue_activated(int);

    void on__min_editingFinished();
    void on__max_editingFinished();

private:
    Ui_DefaultRendererView *ui;

    void setupBandConfiguration(int index);

    Core::LayerRendererConfiguration _conf;

private:
    Core::LayerRenderer * _renderer;

};

//******************************************************************************

}

#endif // DEFAULTRENDERERVIEW_H
