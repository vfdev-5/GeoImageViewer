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
#include "Core/HistogramModel.h"
#include "ColorPalette.h"
#include "ColorPickerFrame.h"


class QComboBox;
class QGraphicsView;
class QRadioButton;

namespace Gui
{

//*************************************************************************

class GIV_DLL_EXPORT HistogramView : public QWidget
{
    Q_OBJECT

public:

    struct Settings
    {
        double margin;
        QPen axisPen;
        QPen dataPen;
        QTransform histogramTransform;

        Settings() :
            margin(0.025),
            axisPen(Qt::black, 0.0),
            dataPen(Qt::gray, 0.0)
        {
        }
    };

    explicit HistogramView(QWidget *parent = 0);

    void setModel(Core::HistogramModel * model);


public slots:
    void clear();

protected:

    void setupUi();
    void drawAxes();
//    void transformAllItems(double newMin, double newMax);

    void showEvent(QShowEvent * event);
    void resizeEvent(QResizeEvent * event);

//    bool eventFilter(QObject *, QEvent *);
//    void setupMenu();

    QGraphicsScene _scene;
    QGraphicsView * _view;
    QComboBox * _histList;
    QRadioButton * _isComplete;
    QRadioButton * _isPartial;



    Settings _settings;

    Core::HistogramModel * _model;

};

//*************************************************************************

}

#endif // HISTOGRAMVIEW_H
