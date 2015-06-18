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
        QWidget(parent)
    {}

    virtual ~AbstractRendererView() {}
    virtual void setup(const Core::ImageRendererConfiguration * renderer, const Core::ImageDataProvider * provider) = 0;

public slots:
    virtual void clear() = 0;
    virtual void revert() = 0;

signals:
    void renderConfigurationChanged(Core::ImageRendererConfiguration *);

protected slots:

protected:

    const Core::ImageDataProvider * _dataProvider;


};

//******************************************************************************

}

#endif // ABSTRACTRENDERERVIEW_H
