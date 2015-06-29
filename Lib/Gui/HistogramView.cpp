
// Qt
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QContextMenuEvent>

// Project
#include "Core/Global.h"
#include "HistogramView.h"


namespace Gui
{

//*************************************************************************

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

QGraphicsSimpleTextItem * createText(const QString & text)
{
    QGraphicsSimpleTextItem * textItem = new QGraphicsSimpleTextItem;
    textItem->setPen(QPen(Qt::black, 0.0));
    textItem->setRotation(90.0);
    textItem->setText(text);
    textItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    QFont f;
    f.setPixelSize(10);
    f.setStyleHint(QFont::System);
    f.setWeight(QFont::Light);
    textItem->setFont(f);
    return textItem;
}

//*************************************************************************

/*!

    \class HistogramView
    \ingroup Gui
    \brief This class implements the histogram visualization widget.
 */

//*************************************************************************

/*!
    Constructor
*/
HistogramView::HistogramView(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui_HistogramView),
    _currentHistogram(0),
    _allBandsHistogram(0),
    _zoomIn(tr("Zoom in"), this),
    _zoomOut(tr("Zoom out"), this),
    _zoomAll(tr("Zoom All"), this)
{
    _ui->setupUi(this);

    // Set scene size:
    _histogramScene.setSceneRect(-_settings.margin,
                                 -_settings.margin,
                                 1.0+2.0*_settings.margin,
                                 1.0+2.0*_settings.margin);
    _visibleZone = _histogramScene.sceneRect();

    // setup context menu
    setupViewContextMenu();

    // Draw items:
    // clear();

    // set histogram scene to the view
    _ui->_histogramView->setStyleSheet("background: dark grey");
    _ui->_histogramView->setScene(&_histogramScene);
    _ui->_histogramView->setRenderHint(QPainter::Antialiasing);
    _ui->_histogramView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    if (_settings.showMinMaxValues)
    {
        _settings.histogramTransform = QTransform::fromScale(1.0, 0.80);
    }

}

//*************************************************************************

/*!
    Destructor
*/
HistogramView::~HistogramView()
{
    clearHistogramItems();
    delete _ui;
}

//*************************************************************************

/*!
   Method to clear all histogram items
 */
void HistogramView::clearHistogramItems()
{
    qDeleteAll(_histograms.begin(), _histograms.end());
    _histograms.clear();

    delete _allBandsHistogram;
    _allBandsHistogram = 0;
}

//*************************************************************************

/*!
    Method to clear all
*/
void HistogramView::clear()
{
    clearHistogramItems();
    _histogramScene.clear();
    _currentHistogram = 0;

    drawAxes();
//    _ui->_histogramView->fitInView(_histogramScene.sceneRect());
    _ui->_histogramView->fitInView(_visibleZone);
}


//*************************************************************************

/*!
 * Method to add histogram data.
 * \param GradientStops are normalized color stops for the ColorPalette
 * \param QVector<double> data is normalized histogram values
*/
void HistogramView::addHistogram(const QVector<double> &data, double xmin, double xmax)
{
    int index = _histograms.size();
    _histograms.append(new HistogramItem());
    if (!setHistogram(index, data, xmin, xmax))
    {
        HistogramItem * h = _histograms.takeLast();
        delete h;
    }
}

//*************************************************************************

/*!
    Method to set histogram data at index.
*/
bool HistogramView::setHistogram(int index, const QVector<double> & data, double xmin, double xmax)
{
    if (index < 0 || index >= _histograms.size())
    {
        return false;
    }
    // draw histogram bars :
    _histograms[index]->graphicsItem = createHistogramGraphicsItem(data, _settings.dataPen);
    _histograms[index]->graphicsItem->setVisible(false);
    _histograms[index]->xmax = xmax;
    _histograms[index]->xmin = xmin;
    // draw xmin/xmax text :
    if (_settings.showMinMaxValues)
    {
        double d = 0.025;
        QGraphicsSimpleTextItem * text1 = createText(QString::number(xmin));
        text1->setPos(QPointF(0.0+d, 1.0+d));
        _histograms[index]->graphicsItem->addToGroup(text1);
        QGraphicsSimpleTextItem * text2 = createText(QString::number(xmax));
        _histograms[index]->graphicsItem->addToGroup(text2);
        text2->setPos(QPointF(1.0+d, 1.0+d));
    }

    _histogramScene.addItem(_histograms[index]->graphicsItem);
    return true;
}

//*************************************************************************
/*!
    Method to draw histogram in gray mode
*/
#define CheckIndex(index) (index < 0 || index >= _histograms.size())

void HistogramView::drawSingleHistogram(int index)
{

    if (CheckIndex(index))
    {
        SD_TRACE("HistogramView::drawSingleHistogram : index is out of bounds");
        return;
    }

    if (_currentHistogram == _histograms[index])
    {
        return;
    }

    // make invisible current histogram:
    if (_currentHistogram && _currentHistogram->graphicsItem)
    {
        _currentHistogram->graphicsItem->setVisible(false);
    }

    _currentHistogram = _histograms[index];

    drawHistogramGraphicsItem(_currentHistogram, _settings.dataPen);

}

//*************************************************************************

/*!
    Method to draw histogram in rgb mode (all 3 histograms)
*/
void HistogramView::drawRgbHistogram(int r, int g, int b)
{

    if (CheckIndex(r) || CheckIndex(g) || CheckIndex(b))
    {
        SD_TRACE("HistogramView::drawRbgHistogram : index is out of bounds");
        return;
    }

    // make invisible current histogram:
    if (_currentHistogram &&
            _currentHistogram->graphicsItem &&
            _currentHistogram != _allBandsHistogram)
    {
        _currentHistogram->graphicsItem->setVisible(false);
    }

    if (!_allBandsHistogram)
    {// if does not exist -> create as default HistogramItem
        _allBandsHistogram = new HistogramItem();
    }

    if (_allBandsHistogram->graphicsItem)
    {// remove children from allBandsHistogram:
        foreach (QGraphicsItem * item, _allBandsHistogram->graphicsItem->childItems())
        {
            _allBandsHistogram->graphicsItem->removeFromGroup(item);
            item->setVisible(false);
        }
    }
    else
    {
        _allBandsHistogram->graphicsItem = new QGraphicsItemGroup();
        _histogramScene.addItem(_allBandsHistogram->graphicsItem);
    }

    _currentHistogram = _allBandsHistogram;

        // draw all histograms:
    QColor dataColors[] = {QColor(255,0,0,81),
                           QColor(0,255,0,81),
                           QColor(0,0,255,81)};
    int indices[] = {r,g,b};
    for (int i=0;i<_histograms.size();i++)
    {
        HistogramItem * h = _histograms[indices[i]];
        drawHistogramGraphicsItem(h, QPen(dataColors[i], 0.0));
        _allBandsHistogram->graphicsItem->addToGroup(h->graphicsItem);
    }
}

//*************************************************************************

/*!
    on Show event
*/
void HistogramView::showEvent(QShowEvent *)
{
    _ui->_histogramView->fitInView(_visibleZone);
}

//*************************************************************************

/*!
    on Resize event
*/
void HistogramView::resizeEvent(QResizeEvent *)
{
    _ui->_histogramView->fitInView(_visibleZone);
}

//*************************************************************************

/*!
    Method to create histogram graphics item
*/
QGraphicsItemGroup * HistogramView::createHistogramGraphicsItem(const QVector<double> &data, const QPen &dataPen)
{
    // Draw histogram image : OX is [0.0; 1.0] / OY is [0.0, 1.0]
    QGraphicsItemGroup *group = new QGraphicsItemGroup();
    int histSize = data.size();
    double d = 0.005;
    for (int i=0; i<histSize; i++)
    {
        double value = data[i];
        QGraphicsLineItem * l = new QGraphicsLineItem(
                    i*1.0/histSize + d,
                    1.0 - value - d,
                    i*1.0/histSize + d,
                    1.0 - d
                    );
        l->setPen(dataPen);
        group->addToGroup(l);
    }
    group->setZValue(0.0);
    return group;
}

//*************************************************************************

/*!
    Method to draw histogram graphics item
*/
QPen getHGIPen(QGraphicsItemGroup * hgi)
{
    QPen p;
    QList<QGraphicsItem*> items = hgi->childItems();
    if (items.isEmpty())
        return p;
    QGraphicsLineItem * line = qgraphicsitem_cast<QGraphicsLineItem *>(items[0]);
    if (line)
        p = line->pen();

    return p;

}

void HistogramView::drawHistogramGraphicsItem(HistogramItem * h, const QPen & dataPen)
{
    h->graphicsItem->setVisible(true);
    if (dataPen != getHGIPen(h->graphicsItem))
    {
        foreach (QGraphicsItem * item, h->graphicsItem->childItems())
        {
            QGraphicsLineItem * line = qgraphicsitem_cast<QGraphicsLineItem *>(item);
            if (line)
                line->setPen(dataPen);
        }
    }

    // intialize histogram bars transform and visible min max as 100 %
    h->graphicsItem->setTransform(_settings.histogramTransform);

}

//*************************************************************************

/*!
    Method to draw the axes
*/
void HistogramView::drawAxes()
{
    // draw axes:
    QGraphicsItemGroup * axes = createAxesGroup(_settings.axisPen);
    _histogramScene.addItem(axes);
    axes->setTransform(_settings.histogramTransform);

}

//*************************************************************************

/*!
  Method to zoom horizontally on the interval vXMin, vXMax of the histogram
 */
void HistogramView::zoomInterval(double vXMin, double vXMax)
{
    SD_TRACE(QString("HistogramView::zoomInterval : %1, %2").arg(vXMin).arg(vXMax));
    if (vXMax < vXMin ||
            vXMax-vXMin < 1.0/_settings.zoomMaxFactor)
    {
        SD_TRACE("HistogramView::zoomInterval : vXMax should be bigger than vXMin");
        return;
    }
    _visibleZone.setX(vXMin);
    _visibleZone.setWidth(vXMax-vXMin);
    _ui->_histogramView->fitInView(_visibleZone);
}

//*************************************************************************

void HistogramView::setupViewContextMenu()
{
    // menu
    _menu.addAction(&_zoomIn);
    _menu.addAction(&_zoomOut);
    _menu.addSeparator();
    _menu.addAction(&_zoomAll);

    connect(&_zoomIn, SIGNAL(triggered()), this, SLOT(onZoomActionTriggered()));
    connect(&_zoomOut, SIGNAL(triggered()), this, SLOT(onZoomActionTriggered()));
    connect(&_zoomAll, SIGNAL(triggered()), this, SLOT(onZoomActionTriggered()));
}

//*************************************************************************

void HistogramView::contextMenuEvent(QContextMenuEvent *event)
{
    if (_histograms.size() > 0)
    {
        _menu.popup(event->globalPos());
    }
    QWidget::contextMenuEvent(event);
}

//*************************************************************************

void HistogramView::zoom(double factor, double xpos)
{
    double width = _visibleZone.width();
    if (width*factor < 1.0/_settings.zoomMaxFactor)
        return;
    if (width*factor >  _histogramScene.sceneRect().width())
    {
        _visibleZone.setX(0);
        _visibleZone.setWidth(_histogramScene.sceneRect().width());
    }
    else
    {
        _visibleZone.setX(xpos - width*factor*0.5);
        _visibleZone.setWidth(width*factor);
    }
    _ui->_histogramView->fitInView(_visibleZone);
}

//*************************************************************************

void HistogramView::onZoomActionTriggered()
{
    QPoint pt = _ui->_histogramView->mapFromGlobal(_menu.pos());
    QPointF scenePt = _ui->_histogramView->mapToScene(pt);
    if (sender() == &_zoomIn)
    {
        zoom(0.5, scenePt.x());
    }
    else if (sender() == &_zoomOut)
    {
        zoom(2.0, scenePt.x());
    }
    else if (sender() == &_zoomAll)
    {
        _visibleZone = _histogramScene.sceneRect();
        _ui->_histogramView->fitInView(_visibleZone);
    }

}

//*************************************************************************

}
