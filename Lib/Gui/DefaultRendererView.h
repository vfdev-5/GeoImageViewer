#ifndef DEFAULTRENDERERVIEW_H
#define DEFAULTRENDERERVIEW_H


// Qt
#include <QWidget>

// Project
#include "ui_DefaultRendererView.h"
#include "AbstractRendererView.h"
#include "Core/ImageRenderer.h"
#include "Core/LibExport.h"

namespace Core {
class ImageDataProvider;
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
    virtual void setup(Core::ImageRenderer * renderer, const Core::ImageDataProvider * provider);
    virtual void applyNewRendererConfiguration();

public slots:
    virtual void clear();

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

//    Core::ImageRenderer * _renderer;

    Core::ImageRendererConfiguration _conf;
    const Core::ImageDataProvider * _dataProvider;

};

//******************************************************************************

}

#endif // DEFAULTRENDERERVIEW_H
