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

    struct HistogramItem
    {
        // graphics item info
        QGraphicsItemGroup * graphicsItem;
        double xmin, xmax;

        HistogramItem() :
            graphicsItem(0),
            xmin(0),
            xmax(255)
        {
        }
    };
public:

    HistogramView(QWidget * parent = 0);
    virtual ~HistogramView();

    struct Settings
    {
        const double margin;
        const QPen axisPen;
        const QPen dataPen;
        QTransform histogramTransform;
        const bool showMinMaxValues;
        Settings() :
            margin(0.025),
            axisPen(Qt::black, 0.0),
            dataPen(Qt::gray, 0.0),
            showMinMaxValues(true)
        {}

    };

    void addHistogram(const QVector<double> &data, double xmin, double xmax);
    bool setHistogram(int index, const QVector<double> & data, double xmin, double xmax);

    void drawSingleHistogram(int index);
    void drawRgbHistogram(int r=0, int g=1, int b=2);


public slots:
    void clear();
//    void resetToDefault();


protected slots:
//    void onDisplayCompleteHist(bool checked);
//    void onDisplayPartialHist(bool checked);
//    void onHistListIndexChanged(int);
//    void onUpdateTimerTimeout();
//    void onDiscreteColorsClicked(bool checked);
//    void onTransferFunctionChanged(QString);
//    void onIsAllBandsClicked(bool checked);


protected:
//    void setTransferFunctionNames(const QStringList & transferFunctionNames);

//    void drawAllHistograms();
    void drawAxes();
    void drawHistogramGraphicsItem(HistogramItem *h, const QPen &dataPen);

//    void transformAllItems(double newMin, double newMax);

    void showEvent(QShowEvent * event);
    void resizeEvent(QResizeEvent * event);

//    bool eventFilter(QObject *, QEvent *);

    QGraphicsItemGroup * createHistogramGraphicsItem(const QVector<double> & data, const QPen &dataPen);

//    void initializeAllBandsHistogram();

    enum Mode {RGB, GRAY};

    Ui_HistogramView *_ui;

    QVector<HistogramItem> _histograms;
    HistogramItem * _currentHistogram;
    HistogramItem _allBandsHistogram;

    QGraphicsScene _histogramScene;
    Settings _settings;

};

//*************************************************************************

}

#endif // HISTOGRAMVIEW_H
