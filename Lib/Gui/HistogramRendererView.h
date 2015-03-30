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

// Opencv
#include <opencv2/core/core.hpp>


// Project
#include "Core/LibExport.h"
#include "Core/HistogramImageRenderer.h"
#include "ui_HistogramRendererView.h"
#include "AbstractRendererView.h"
#include "ColorPalette.h"
#include "ColorPickerFrame.h"

namespace Core {
class ImageDataProvider;
}

namespace Gui
{

//*************************************************************************

class GIV_DLL_EXPORT HistogramRendererView : public AbstractRendererView
{
    Q_OBJECT

    struct HistogramDataView
    {
        // histogram data description
        double xmin, xmax; // Global histogram min/max
        double xmin2, xmax2; // 95% quantiles
        int bandId;

        // graphics item info
        QGraphicsItemGroup * graphicsItem;
        double vxmin;
        double vxmax;

        // slider positions (real image values) & transfer function & isDiscrete
        QGradientStops outputStops;
        QString transferFunctionName;
        bool isDiscrete;

        HistogramDataView() :
            xmin(-12345),
            xmax(-12345),
            xmin2(-12345),
            xmax2(-12345),
            bandId(-1),
            graphicsItem(0),
            vxmin(-12345),
            vxmax(-12345),
            isDiscrete(false)
        {
        }
    };


public:

    struct Settings
    {
        double margin;
        double histOverPaletteRatio;
        QPen axisPen;
        QPen dataPen;
        QTransform histogramTransform;
        QTransform colorPaletteTransform;
        int updateDelayMaxTime;


        Settings() :
            margin(0.025),
            axisPen(Qt::black, 0.0),
            dataPen(Qt::gray, 0.0),
            histOverPaletteRatio(0.75),
            updateDelayMaxTime(250)
        {
        }
    };


public:
    explicit HistogramRendererView(QWidget *parent = 0);
    virtual ~HistogramRendererView();

    virtual void setup(Core::ImageRenderer * renderer, const Core::ImageDataProvider * provider);
    virtual void applyNewRendererConfiguration();

protected:

    void addHistogram(int id, const QString & name, const QVector<double> &data,
                      const Core::ImageRendererConfiguration & conf, const Core::HistogramRendererConfiguration & histConf);
    void drawHistogram();
    void setTransferFunctionNames(const QStringList & transferFunctionNames);

public slots:
    virtual void clear();
    void resetToDefault();
    void onSliderPositionChanged(int index, double position);

protected slots:
    // Slider management
    void onRemoveSlider();
    void onAddSlider();
    void onRevertSlider();
    void onColorPicked(QColor);
    void onValueEdited();

    void onDisplayCompleteHist(bool checked);
    void onDisplayPartialHist(bool checked);
    void onHistListIndexChanged(int);
    void onUpdateTimerTimeout();
    void onDiscreteColorsClicked(bool checked);
    void onTransferFunctionChanged(QString);
    void onIsAllBandsClicked(bool checked);


    void on__redChannel_editingFinished();
    void on__greenChannel_editingFinished();
    void on__blueChannel_editingFinished();

    void on__isGrayMode_toggled();
    void on__isRgbMode_toggled();

protected:

    void updateHistogramData(int index=-1, double xpos = -13245, const QColor &c = QColor(), bool notificationDelayed=false);
    void drawHistogram(int index);
    void drawAllHistograms();
    void drawAxes();
    void createColorPalette();
    void drawColorPalette(const QGradientStops & houtputStops, double vxmin, double vxmax, bool isDiscrete);

    void transformAllItems(double newMin, double newMax);

    void showEvent(QShowEvent * event);
    void resizeEvent(QResizeEvent * event);

    bool eventFilter(QObject *, QEvent *);

    void setupMenu();

private:

    QGraphicsItemGroup * createHistogramGraphicsItem(const QVector<double> & data, const QPen &dataPen);
    void drawHistogramGraphicsItem(HistogramDataView *h, const QPen &dataPen);
    void initializeAllBandsHistogram();

    enum Mode {RGB, GRAY};

    Ui_HistogramRendererView *_ui;

    QVector<HistogramDataView> _histograms;
    HistogramDataView * _currentHistogram;
    HistogramDataView _allBandsHistogram;

    QGraphicsScene _histogramScene;
    Settings _settings;

    ColorPalette * _colorPalette;
    QList<QGraphicsLineItem*> _sliderLines;

    int _indexOfActionedSlider;

    QMenu _menu;
    QAction _removeSliderAction;
    QAction _addSliderAction;
    QAction _revertSliderAction;

    ColorPickerFrame _colorPicker;
    QLineEdit _valueEditor;

    Mode _mode;
    QTimer _updateDelayTimer;

//    Core::HistogramImageRenderer * _hRenderer;

};


//*************************************************************************

} // namespace Gui

#endif // HISTOGRAMRENDERERVIEW_H
