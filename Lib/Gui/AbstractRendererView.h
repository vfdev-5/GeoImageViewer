#ifndef ABSTRACTRENDERERVIEW_H
#define ABSTRACTRENDERERVIEW_H


// Qt
#include <QWidget>

// Project
#include "Core/LibExport.h"
#include "Core/ImageRenderer.h"

namespace Core {
class ImageDataProvider;
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
        _renderer(0)
    {
    }

    virtual ~AbstractRendererView() {}
    virtual void setup(Core::ImageRenderer * renderer, const Core::ImageDataProvider * provider) = 0;
    virtual void applyNewRendererConfiguration() = 0;

    const Core::ImageRenderer * getRenderer() const
    { return _renderer; }

public slots:
    virtual void clear() = 0;

signals:
    void renderConfigurationChanged();

protected slots:
    virtual void onRendererDestroyed(QObject * rObject)
    {
        _renderer = 0;
        clear();
    }

protected:
    void setupRenderer(Core::ImageRenderer * renderer)
    {
        if (_renderer)
        {
            disconnect(_renderer, SIGNAL(destroyed(QObject*)), this, SLOT(onRendererDestroyed(QObject*)));
        }
        _renderer = renderer;
        connect(_renderer, SIGNAL(destroyed(QObject*)), this, SLOT(onRendererDestroyed(QObject*)));
    }

    // can be modified with applyNewRendererConfiguration()
    Core::ImageRenderer * _renderer;


};

//******************************************************************************

}

#endif // ABSTRACTRENDERERVIEW_H
