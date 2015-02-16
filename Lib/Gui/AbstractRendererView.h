#ifndef ABSTRACTRENDERERVIEW_H
#define ABSTRACTRENDERERVIEW_H


// Qt
#include <QWidget>

// Project
#include "Core/LibExport.h"
#include "Core/LayerRenderer.h"

namespace Core {
class ImageLayer;
}


namespace Gui
{

//******************************************************************************

class GIV_DLL_EXPORT AbstractRendererView : public QWidget
{
    Q_OBJECT
public:
    AbstractRendererView(QWidget * parent = 0) :
        QWidget(parent),
        _currentLayer(0)
    {
    }

    virtual ~AbstractRendererView() {}
    virtual void setup(Core::LayerRenderer * renderer, const Core::ImageLayer * layer) = 0;
    virtual void applyNewRendererConfiguration() = 0;

public slots:
    virtual void clear() = 0;

signals:
    void renderConfigurationChanged();

protected:
    const Core::ImageLayer * _currentLayer;

};

//******************************************************************************

}

#endif // ABSTRACTRENDERERVIEW_H
