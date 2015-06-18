#ifndef HISTOGRAMVIEW_H
#define HISTOGRAMVIEW_H


// Qt
#include <QWidget>
#include <QGraphicsScene>
#include <QMenu>
#include <QLineEdit>
#include <QTimer>

// Project
#include "Core/LibExport.h"
#include "ui_HistogramView.h"
#include "ColorPalette.h"
#include "ColorPickerFrame.h"

namespace Gui
{

//*************************************************************************

class HistogramView : public QWidget
{
    Q_OBJECT


    struct Data
    {
        // histogram data description
        double xmin, xmax; // Global histogram min/max
        double xmin2, xmax2; // 95% quantiles
        int bandId;
        // slider positions (real image values) & isDiscrete
        QGradientStops outputStops;
        bool isDiscrete;
        Data() :
            xmin(-12345),
            xmax(-12345),
            xmin2(-12345),
            xmax2(-12345),
            bandId(-1),
            isDiscrete(false)
        {}
    };
    struct GraphicsItem
    {
        // graphics item info
        QGraphicsItemGroup * graphicsItem;
        double vxmin;
        double vxmax;
        GraphicsItem() :
           graphicsItem(0),
           vxmin(-12345),
           vxmax(-12345)
        {}
    };

    struct HistogramDataView
    {
        // histogram data description
//        double xmin, xmax; // Global histogram min/max
//        double xmin2, xmax2; // 95% quantiles
//        int bandId;

        // graphics item info
        QGraphicsItemGroup * graphicsItem;
        double vxmin;
        double vxmax;

        // slider positions (real image values) & transfer function & isDiscrete
        QGradientStops outputStops;
//        QString transferFunctionName;
        bool isDiscrete;

        HistogramDataView() :
//            xmin(-12345),
//            xmax(-12345),
//            xmin2(-12345),
//            xmax2(-12345),
//            bandId(-1),
            graphicsItem(0),
            vxmin(-12345),
            vxmax(-12345),
            isDiscrete(false)
        {
        }
    };
public:

    HistogramView(QWidget * parent = 0);
    virtual ~HistogramView();

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
        {}
    };


    void addHistogram(const QGradientStops &normStops, const QVector<double> &data);
    bool setHistogram(int index, const QGradientStops & normHistStops, const QVector<double> & data);

    void drawSingleHistogram(int index);
    void drawRGBHistogram(int r, int g, int b) {}

//    void addHistogram(int id, const QString & name, const QVector<double> &data);
//    void drawHistogram();

public slots:
    void clear();
//    void resetToDefault();
//    void onSliderPositionChanged(int index, double position);


protected slots:
    // Slider management
    void onRemoveSlider();
    void onAddSlider();
    void onRevertSlider();
    void onColorPicked(QColor);
    void onValueEdited();

//    void onDisplayCompleteHist(bool checked);
//    void onDisplayPartialHist(bool checked);
//    void onHistListIndexChanged(int);
//    void onUpdateTimerTimeout();
//    void onDiscreteColorsClicked(bool checked);
//    void onTransferFunctionChanged(QString);
//    void onIsAllBandsClicked(bool checked);


protected:
//    void setTransferFunctionNames(const QStringList & transferFunctionNames);

    void updateHistogramData(int index=-1, double xpos = -13245, const QColor &c = QColor(), bool notificationDelayed=false);
//    void drawAllHistograms();
    void drawAxes();
    void createColorPalette();
    void drawHistogramGraphicsItem(HistogramDataView *h, const QPen &dataPen);
    void drawColorPalette(const QGradientStops & houtputStops, double vxmin, double vxmax, bool isDiscrete);

//    void transformAllItems(double newMin, double newMax);

//    void showEvent(QShowEvent * event);
//    void resizeEvent(QResizeEvent * event);

//    bool eventFilter(QObject *, QEvent *);
    void setupMenu();


    QGraphicsItemGroup * createHistogramGraphicsItem(const QVector<double> & data, const QPen &dataPen);

//    void initializeAllBandsHistogram();

    enum Mode {RGB, GRAY};

    Ui_HistogramView *_ui;

//    QVector<HistogramDataView> _histograms;


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

//    Mode _mode;
    QTimer _updateDelayTimer;

};

//*************************************************************************

}

#endif // HISTOGRAMVIEW_H
