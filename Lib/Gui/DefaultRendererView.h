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
//    virtual void setup(Core::ImageRenderer * renderer, const Core::ImageDataProvider * provider);
    virtual void setup(const Core::ImageRendererConfiguration & conf, const Core::ImageDataProvider * provider);
//    virtual void applyNewRendererConfiguration();

public slots:
    virtual void clear();
    virtual void revert();

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

    Core::ImageRendererConfiguration _conf;
    Core::ImageRendererConfiguration _initialConf;

};

//******************************************************************************

}

#endif // DEFAULTRENDERERVIEW_H
