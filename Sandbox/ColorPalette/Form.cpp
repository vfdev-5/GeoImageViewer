

// Project
#include "Form.h"
#include "ui_Form.h"
#include "Gui/ColorPalette.h"

//*************************************************************************

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    _scene.setSceneRect(-0.15, -0.15, 1.30, 1.30);
    ui->_view->setScene(&_scene);


    // Setup color palette
    Gui::ColorPalette * colorPalette = new Gui::ColorPalette();
    _scene.addItem(colorPalette);


    QGradientStops stops = QGradientStops()
            << QGradientStop(0.1, QColor(Qt::black))
            << QGradientStop(0.3, QColor(Qt::blue))
//            << QGradientStop(0.4, QColor(Qt::blue))
//            << QGradientStop(0.5, QColor(Qt::blue))
//            << QGradientStop(0.6, QColor(Qt::red))
            << QGradientStop(0.7, QColor(Qt::red))
            << QGradientStop(0.9, QColor(Qt::green));

    double xmin = -150.5;
    double xmax = 243.7;
    colorPalette->setupPalette(stops, xmin, xmax, false);
    colorPalette->setZValue(0.1);


//    QGraphicsItem * item = _scene.addRect(_scene.sceneRect(), QPen(Qt::black,0.0), QBrush(Qt::magenta));
//    item->setZValue(0.2);



}

//*************************************************************************

Form::~Form()
{
    delete ui;
}

//*************************************************************************

/*!
    on Show event
*/
void Form::showEvent(QShowEvent *)
{
    ui->_view->fitInView(_scene.sceneRect());
}

//*************************************************************************

/*!
    on Resize event
*/
void Form::resizeEvent(QResizeEvent *)
{
    ui->_view->fitInView(_scene.sceneRect());
}

//*************************************************************************
