
// Qt
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QComboBox>
#include <QRadioButton>

// Project
#include "HistogramView.h"


namespace Gui
{

namespace AA {

QGraphicsItemGroup * createAxesGroup(const QPen & pen)
{
    QGraphicsItemGroup * axes = new QGraphicsItemGroup();
    QGraphicsLineItem * axisX = new QGraphicsLineItem(
                0.0, 1.0, 1.0, 1.0
                );
    axisX->setPen(pen);
    axes->addToGroup(axisX);

    QGraphicsLineItem * axisY = new QGraphicsLineItem(
                0.0, 0.0, 0.0, 1.0
                );
    axisY->setPen(pen);
    axes->addToGroup(axisY);
    return axes;
}

}

//*************************************************************************

HistogramView::HistogramView(QWidget *parent) :
    QWidget(parent),
    _view(new QGraphicsView())
{

    setupUi();

    // Set scene size:
    _scene.setSceneRect(-_settings.margin,
                        -_settings.margin,
                        1.0+2.0*_settings.margin,
                        1.0+2.0*_settings.margin);
    _scene.installEventFilter(this);

    _settings.histogramTransform = QTransform::fromScale(1.0,1.0);

    // Draw items:
    clear();

    // set histogram scene to the view
    _view->setStyleSheet("background: dark grey");
    _view->setScene(&_scene);
    _view->setRenderHint(QPainter::Antialiasing);

}

//*************************************************************************

void HistogramView::setupUi()
{
    // setup view:
    setLayout(new QVBoxLayout());
    layout()->addWidget(_view);

    // setup band chooser
    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->addWidget(new QLabel(tr("Histogram source :"), this));
    hlayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    _histList = new QComboBox(this);
    hlayout->addWidget(_histList);

//    _isAllBands = new QCheckBox(HistogramRendererView);
//    _isAllBands->setObjectName(QStringLiteral("_isAllBands"));
//    _isAllBands->setEnabled(false);
//    horizontalLayout->addWidget(_isAllBands);

    // setup display options
    QHBoxLayout * hlayout2 = new QHBoxLayout();
    hlayout2->addWidget(new QLabel(tr("Display histogram :"), this));
    _isComplete = new QRadioButton(tr("100%"), this);
    _isPartial = new QRadioButton(tr("100%"), this);
//    hlayout2->addWidget();


    layout()->addItem(hlayout);
}

//*************************************************************************

/*!
    Method to clear all
*/
void HistogramView::clear()
{
    _scene.clear();

//    _ui->_isPartial->setEnabled(false);
//    _ui->_isComplete->setEnabled(false);

    drawAxes();
    _view->fitInView(_scene.sceneRect());
}

//*************************************************************************

/*!
    Method to draw the axes
*/
void HistogramView::drawAxes()
{
    // draw axes:
    QGraphicsItemGroup * axes = AA::createAxesGroup(_settings.axisPen);
    _scene.addItem(axes);
    axes->setTransform(_settings.histogramTransform);
}

//*************************************************************************

/*!
    on Show event
*/
void HistogramView::showEvent(QShowEvent * event)
{
    if (event->type() == QEvent::Show)
    {
        _view->fitInView(_scene.sceneRect());
    }
}

//*************************************************************************

/*!
    on Resize event
*/
void HistogramView::resizeEvent(QResizeEvent * event)
{
    if (event->type() == QEvent::Resize)
    {
        _view->fitInView(_scene.sceneRect());
    }
}

//*************************************************************************

void HistogramView::setModel(Core::HistogramModel *model)
{
    _model = model;
    if (!_model)
        return;




}

//*************************************************************************

}
