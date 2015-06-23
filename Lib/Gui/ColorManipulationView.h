#ifndef COLORMANIPULATIONVIEW_H
#define COLORMANIPULATIONVIEW_H


// Qt
#include <QWidget>
#include <QGraphicsScene>
#include <QMenu>
#include <QLineEdit>
#include <QTimer>

// Project
#include "Core/LibExport.h"
#include "HistogramView.h"
#include "ColorPalette.h"
#include "ColorPickerFrame.h"

namespace Gui
{

//*************************************************************************

class ColorManipulationView : public HistogramView
{
    Q_OBJECT

public:

    ColorManipulationView(QWidget * parent = 0);
//    virtual ~ColorManipulationView() {}

    struct CMVSettings
    {
        const double histOverPaletteRatio;
        QTransform colorPaletteTransform;
        int updateDelayMaxTime;
        bool interactiveColorPalette;
        CMVSettings() :
            histOverPaletteRatio(0.75),
            updateDelayMaxTime(250),
            interactiveColorPalette(true)
        {}
    };

    void addHistogram(const QGradientStops &normStops, const QVector<double> &data, double xmin, double xmax);
    bool setHistogram(int index, const QGradientStops & normHistStops, const QVector<double> & data, double xmin, double xmax);

    void drawSingleHistogram(int index);
    void drawRgbHistogram(int r=0, int g=1, int b=2);

signals:
    void stopsChanged(QGradientStops stops);

public slots:
    void clear();
    void onSliderPositionChanged(int index, double position);
    void onSliderMouseRelease(int index, double position);
    void onDiscreteColorsClicked(bool checked);

protected slots:
    // Slider management
    void onRemoveSlider();
    void onAddSlider();
    void onRevertSlider();
    void onColorPicked(QColor);
    void onValueEdited();
    void onFitSliders();

    virtual void onContextMenuRequested(QPoint p);


protected:

    struct CMVHistogramItem : public HistogramItem
    {
        // slider positions (normalized values) & isDiscrete
        QGradientStops outputStops;
        bool isDiscrete;

        CMVHistogramItem() :
            HistogramItem(),
            isDiscrete(false)
        {
        }

    };

    void updateHistogramItem(int index=-1, double xpos = -13245, const QColor &c = QColor(), bool notificationDelayed=false);
    void createColorPalette();
    void drawColorPalette(const QGradientStops & houtputStops, double xmin, double xmax, bool isDiscrete);

    bool eventFilter(QObject *, QEvent *);
    void valueEditorEvents(QEvent *);
    void setupViewContextMenu();

    ColorPalette * _colorPalette;
    QList<QGraphicsLineItem*> _sliderLines;

    int _indexOfActionedSlider;

    QAction _removeSlider;
    QAction _addSlider;
    QAction _revertSlider;
    QAction _zoomFitSliders;

    ColorPickerFrame _colorPicker;
    QLineEdit _valueEditor;

    CMVSettings _cmvSettings;
};

//*************************************************************************

}

#endif // COLORMANIPULATIONVIEW_H
