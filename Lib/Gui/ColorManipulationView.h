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

namespace Gui
{

//*************************************************************************

class GIV_DLL_EXPORT ColorManipulationView : public HistogramView
{
    Q_OBJECT

public:

    explicit ColorManipulationView(QWidget * parent = 0);
//    virtual ~ColorManipulationView() {}

    struct CMVSettings
    {
        const double histOverPaletteRatio;
        QTransform colorPaletteTransform;
        QTransform colorPaletteTransform2;
        int updateDelayMaxTime;
        bool interactiveColorPalette;
        CMVSettings() :
            histOverPaletteRatio(0.75),
            updateDelayMaxTime(250),
            interactiveColorPalette(true)
        {}
    };

    void addHistogram(const QGradientStops &normStops, const QVector<double> &data, double xmin, double xmax, bool isDiscrete);
    bool setHistogram(int index, const QGradientStops & normHistStops, const QVector<double> & data, double xmin, double xmax, bool isDiscrete);

    void drawSingleHistogram(int index);
    void drawRgbHistogram(int r=0, int g=1, int b=2);

signals:
    void stopsChanged(int hIndex, QGradientStops stops);

public slots:
    void clear();
    void setDiscreteColors(bool value);

protected slots:
    // Slider management
    void onRemoveSlider();
    void onAddSlider(Slider *slider);
    void onRevertSlider();
    void onSliderPositionChanged(Slider *slider, double position);
    void onSliderColorChanged(Slider* slider, const QColor & c);
    // Zoom
    void onFitSliders();

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

    CMVHistogramItem * getHistogramItem(ColorPalette * colorPalette);

    void updateAllStops(ColorPalette * colorPalette);
//    void updateHistogramItem(int index=-1, double xpos = -13245, const QColor &c = QColor(), bool notificationDelayed=false);
    ColorPalette *createColorPalette(const QTransform & transform = QTransform());
    QGraphicsLineItem * createSliderLine(Slider * slider);
    void setupColorPalette(ColorPalette* palette, const QGradientStops & houtputStops, double xmin, double xmax, bool isDiscrete);

    void removeColorPalettes();

    void setupContextMenu();

    QMap<ColorPalette*, CMVHistogramItem*> _paletteHistogramMap;
    QList<ColorPalette*> _colorPalettes;
//    ColorPalette* _currentColorPalette;

    QAction _zoomFitSliders;

    CMVSettings _cmvSettings;
};

//*************************************************************************

}

#endif // COLORMANIPULATIONVIEW_H
