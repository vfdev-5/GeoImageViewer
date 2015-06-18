#ifndef HISTOGRAMRENDERERVIEW_H
#define HISTOGRAMRENDERERVIEW_H

// Qt
#include <QWidget>
#include <QGraphicsScene>
#include <QPen>
#include <QShowEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QLineEdit>
#include <QTimer>
#include <QVector>

// Project
#include "Core/LibExport.h"
#include "Core/HistogramImageRenderer.h"
#include "AbstractRendererView.h"
#include "ui_HistogramRendererView.h"

namespace Core {
class ImageDataProvider;
}

namespace Gui
{

//*************************************************************************

class GIV_DLL_EXPORT HistogramRendererView : public AbstractRendererView
{
    Q_OBJECT

public:
    explicit HistogramRendererView(QWidget *parent = 0);
    virtual ~HistogramRendererView();

    virtual void setup(const Core::ImageRendererConfiguration * conf, const Core::ImageDataProvider * provider);

public slots:
    virtual void clear();
    virtual void revert();

protected slots:

    void on__redChannel_editingFinished();
    void on__greenChannel_editingFinished();
    void on__blueChannel_editingFinished();
    void on__isGrayMode_toggled();
    void on__isRgbMode_toggled();

protected:

    void setRGBModeEnabled(bool value);

    Ui_HistogramRendererView * _ui;

    Core::HistogramRendererConfiguration _conf;
    Core::HistogramRendererConfiguration _initialConf;

};


//*************************************************************************

} // namespace Gui

#endif // HISTOGRAMRENDERERVIEW_H
