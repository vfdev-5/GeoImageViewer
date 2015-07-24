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

public:

    HistogramView(QWidget * parent = 0);
    virtual ~HistogramView();

    struct HVSettings
    {
        const double margin;
        const QPen axisPen;
        const QPen dataPen;
        QTransform histogramTransform;
        const bool showMinMaxValues;
        const double zoomMaxFactor;
        HVSettings() :
            margin(0.025),
            axisPen(Qt::black, 0.0),
            dataPen(Qt::gray, 0.0),
            showMinMaxValues(false),
            zoomMaxFactor(75)
        {}

    };

    void addHistogram(const QVector<double> &data, double xmin, double xmax);
    bool setHistogram(int index, const QVector<double> & data, double xmin, double xmax);

    void drawSingleHistogram(int index);
    void drawRgbHistogram(int r=0, int g=1, int b=2);

    void zoomInterval(double vXMin, double vXMax);
    void zoomAll();

public slots:
    void clear();

protected slots:
    void onZoomActionTriggered();

protected:

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

    void drawAxes();
    void drawHistogramGraphicsItem(HistogramItem *h, const QPen &dataPen);
    void setupContextMenu();

    void showEvent(QShowEvent * event);
    void resizeEvent(QResizeEvent * event);
    virtual void contextMenuEvent(QContextMenuEvent * event);

    void zoom(double factor, double xpos);

    Ui_HistogramView *_ui;

    QMenu _menu;
    QAction _zoomIn;
    QAction _zoomOut;
    QAction _zoomAll;

    void clearHistogramItems();
    QList<HistogramItem*> _histograms;
    HistogramItem * _currentHistogram;
    HistogramItem * _allBandsHistogram;

    QGraphicsScene _histogramScene;
    QRectF _visibleZone;

    HVSettings _settings;
    QGraphicsItemGroup * createHistogramGraphicsItem(const QVector<double> & data, const QPen &dataPen);

};

//*************************************************************************

}

#endif // HISTOGRAMVIEW_H
