
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
#include <QSpinBox>

// Project
#include "Core/Global.h"
#include "HistogramView.h"


namespace Gui
{

//*************************************************************************

QGraphicsLineItem * createSliderLine()
{
    QGraphicsLineItem * line = new QGraphicsLineItem(
                0.0,0.0,
                0.0,1.0
                );
    line->setPen(QPen(QBrush(Qt::white), 0.0, Qt::DashLine));
    line->setZValue(2.0);
    return line;
}

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

inline double normalized(double x, double xmin, double xmax)
{
    return (x - xmin)/(xmax - xmin);
}

inline double unnormalized(double x, double xmin, double xmax)
{
    return x*(xmax - xmin) + xmin;
}

inline void updateToPartialMode(QGraphicsItem * hgi, double oldMin, double oldMax, double newMin, double newMax)
{
    hgi->setTransform(
                QTransform::fromTranslate(-normalized(newMin, oldMin, oldMax),0) *
                QTransform::fromScale((oldMax - oldMin)/(newMax - newMin),1)
                );
}

inline void copyPositions(const QGradientStops & src, QGradientStops & dst)
{
    if (src.size() != dst.size()) return;
    for(int i=0;i<src.size();i++)
    {
        dst[i].first = src[i].first;
    }
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
    _currentHistogram(0)
{
    _ui->setupUi(this);

    //    connect(_ui->_isPartial, SIGNAL(clicked(bool)), this, SLOT(onDisplayPartialHist(bool)));
    //    connect(_ui->_isComplete, SIGNAL(clicked(bool)), this, SLOT(onDisplayCompleteHist(bool)));

    // Set scene size:
    _histogramScene.setSceneRect(-_settings.margin,
                                 -_settings.margin,
                                 1.0+2.0*_settings.margin,
                                 1.0+2.0*_settings.margin);
    _histogramScene.installEventFilter(this);

    // Draw items:
    // clear();

    // set histogram scene to the view
    _ui->_histogramView->setStyleSheet("background: dark grey");
    _ui->_histogramView->setScene(&_histogramScene);
    _ui->_histogramView->setRenderHint(QPainter::Antialiasing);


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
    delete _ui;
}

//*************************************************************************

/*!
    Method to clear all
*/
void HistogramView::clear()
{
    _histograms.clear();
    _allBandsHistogram.graphicsItem=0;

    _histogramScene.clear();
    _currentHistogram = 0;

    drawAxes();
    _ui->_histogramView->fitInView(_histogramScene.sceneRect());
}

//*************************************************************************

///*!
//    Method to reset sliders to default values
//*/
//void HistogramView::resetToDefault()
//{
//    if (!_currentHistogram)
//        return;

//    if (_colorPalette)
//    {
//        // remove color palette with sliders
//        _histogramScene.removeItem(_colorPalette);
//    }
//    // redraw color palette & sliders & sliderlines
//    createColorPalette();

//    _ui->_transferFunction->setCurrentIndex(0);
//    _currentHistogram->transferFunctionName=_ui->_transferFunction->currentIndex();
//    _ui->_discreteColors->setChecked(false);
//    _currentHistogram->isDiscrete=_ui->_discreteColors->isChecked();

//    // reset current histogram outputstops
//    if (_currentHistogram->bandId > -1)
//    {
//        int index = _ui->_histList->currentIndex();
//        double a=(_currentHistogram->xmax2 - _currentHistogram->xmin2);
//        double b=_currentHistogram->xmin2;
//        if (a == 0)
//            a = 1;
//        if (_mode == RGB)
//            _currentHistogram->outputStops = Core::resetStops(index,a,b);
//        else
//            _currentHistogram->outputStops = Core::resetStops(-1,a,b);
//        // redraw histogram and sliders:
//        drawHistogram(index);
//    }
//    else
//    {
//        double a=(_currentHistogram->xmax2 - _currentHistogram->xmin2);
//        double b=_currentHistogram->xmin2;
//        _currentHistogram->outputStops = Core::resetStops(4,a,b);
//        // redraw
//        drawAllHistograms();
//    }


//    // notify
//    emit renderConfigurationChanged(_conf);

//}

////*************************************************************************

//void HistogramView::initializeAllBandsHistogram()
//{
//    _allBandsHistogram.bandId=-1;
//    double xmin(1e10), xmax(-1e10), xmin2(xmin), xmax2(xmax);
//    foreach (HistogramItem h, _histograms)
//    {
//        if (h.xmin < xmin) xmin = h.xmin;
//        if (h.xmax > xmax) xmax = h.xmax;
//        if (h.xmin2 < xmin2) xmin2 = h.xmin2;
//        if (h.xmax2 > xmax2) xmax2 = h.xmax2;

//    }
//    _allBandsHistogram.xmin=xmin;
//    _allBandsHistogram.xmin2=xmin2;
//    _allBandsHistogram.xmax=xmax;
//    _allBandsHistogram.xmax2=xmax2;
//    _allBandsHistogram.vxmin = _allBandsHistogram.xmin;
//    _allBandsHistogram.vxmax = _allBandsHistogram.xmax;
//    _allBandsHistogram.isDiscrete=false;
//    _allBandsHistogram.outputStops=Core::resetStops(4,xmax2-xmin2, xmin2);
//    _allBandsHistogram.transferFunctionName=_ui->_transferFunction->itemText(0);
//    _allBandsHistogram.graphicsItem=new QGraphicsItemGroup();
//    _allBandsHistogram.graphicsItem->setVisible(true);
//    _histogramScene.addItem(_allBandsHistogram.graphicsItem);

//}

////*************************************************************************
///*!
// * \brief HistogramView::setup
// * \param renderer
// * \param layer
// */
//void HistogramView::setup(const Core::ImageRendererConfiguration &conf, const Core::ImageDataProvider * provider)
//{
//    const Core::HistogramRendererConfiguration * test = static_cast<const Core::HistogramRendererConfiguration*>(&conf);
//    if (!test)
//    {
//        SD_TRACE("HistogramView::setup : Failed to cast ImageRendererConfiguration into HistogramRendererConfiguration");
//        return;
//    }
//    _conf = *test;
//    _initialConf = _conf;

//    int nbBands = _conf.normHistStops.size();

//    // configure toRGBMapping frame:
//    if (_conf.mode == Core::HistogramRendererConfiguration::RGB)
//    {
//        int nbBands = _conf.normHistStops.size();
//        _ui->_toRGBMapping->setVisible(true);
//        configureAChannel(_ui->_redChannel, _conf.toRGBMapping[0]+1, 1, nbBands);
//        configureAChannel(_ui->_greenChannel, _conf.toRGBMapping[1]+1, 1, nbBands);
//        configureAChannel(_ui->_blueChannel, _conf.toRGBMapping[2]+1, 1, nbBands);
//        configureAChannel(_ui->_grayChannel, _conf.toRGBMapping[0]+1, 1, nbBands);
//        _mode = RGB;
//    }
//    else
//    {
//        _ui->_toRGBMapping->setVisible(false);
//        _mode = GRAY;
//    }

//    // create histogram views
//    // 1) 1 band layer -> mode=GRAY, 1 histogram
//    // 2) Complex M bands layer -> mode=GRAY, 1 of 4*M histograms
//    // 3) Non-complex N bands layer -> { mode=RGB, 3 histograms | mode=GRAY, 1 of N histograms }

//    nbBands = qMin(3, nbBands);
//    for (int i=0; i<nbBands;i++)
//    {
//        int index = conf.toRGBMapping[i];
//        addHistogram(index, provider->getBandNames()[index],
//                     provider->getBandHistograms()[index]);
//    }

//    drawHistogram();
//}

//*************************************************************************

/*!
 * Method to add histogram data.
 * \param GradientStops are normalized color stops for the ColorPalette
 * \param QVector<double> data is normalized histogram values
*/
void HistogramView::addHistogram(const QVector<double> &data, double xmin, double xmax)
{
    int index = _histograms.size();
    _histograms.resize(index+1);
    setHistogram(index, data, xmin, xmax);
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
    _histograms[index].graphicsItem = createHistogramGraphicsItem(data, _settings.dataPen);
    _histograms[index].graphicsItem->setVisible(false);
    _histograms[index].xmax = xmax;
    _histograms[index].xmin = xmin;
    // draw xmin/xmax text :
    if (_settings.showMinMaxValues)
    {
        double d = 0.025;
        QGraphicsSimpleTextItem * text1 = createText(QString::number(xmin));
        text1->setPos(QPointF(0.0+d, 1.0+d));
        _histograms[index].graphicsItem->addToGroup(text1);
        QGraphicsSimpleTextItem * text2 = createText(QString::number(xmax));
        _histograms[index].graphicsItem->addToGroup(text2);
        text2->setPos(QPointF(1.0+d, 1.0+d));
    }

    _histogramScene.addItem(_histograms[index].graphicsItem);
    return true;
}


////*************************************************************************

///*!
//    Method to add histogram data. For single band display, add only one histogram. For RGB display, add exactly 3 histograms.
//*/
//void HistogramView::addHistogram(int id, const QString &name, const QVector<double> & data)
//{
//    _ui->_histList->addItem(name, id);

//    int index = _histograms.size();
//    _histograms.resize(index+1);
//    _histograms[index].bandId=id;
//    _histograms[index].xmin=_conf.minValues[id];
//    _histograms[index].xmax=_conf.maxValues[id];
//    _histograms[index].xmin2=_conf.qMinValues[id];
//    _histograms[index].xmax2=_conf.qMaxValues[id];
//    _histograms[index].transferFunctionName=_conf.transferFunctions[id]->getName();
//    _histograms[index].isDiscrete=_conf.isDiscreteValues[id];

//    if (_conf.mode == Core::HistogramRendererConfiguration::GRAY)
//    {
//        _histograms[index].outputStops=computeValuesFromStops(_conf.normHistStops[id], _histograms[index].xmin, _histograms[index].xmax);
//    }
//    else if (_conf.mode == Core::HistogramRendererConfiguration::RGB)
//    {
//        _histograms[index].outputStops=computeValuesFromStops(_conf.normRGBHistStops[id], _histograms[index].xmin, _histograms[index].xmax);
//    }

//    // draw histogram bars :
//    _histograms[index].graphicsItem = createHistogramGraphicsItem(data, _settings.dataPen);
//    _histograms[index].graphicsItem->setVisible(false);
//    _histogramScene.addItem(_histograms[index].graphicsItem);

//}

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

    if (_currentHistogram == &_histograms[index])
    {
        return;
    }

    // make invisible current histogram:
    if (_currentHistogram && _currentHistogram->graphicsItem)
    {
        _currentHistogram->graphicsItem->setVisible(false);
    }

    _currentHistogram = &_histograms[index];

    drawHistogramGraphicsItem(_currentHistogram, _settings.dataPen);

    // default choice is 95% of histogram to display
//    _ui->_isPartial->setChecked(true);
//    onDisplayPartialHist(true);

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
            _currentHistogram != &_allBandsHistogram)
    {
        _currentHistogram->graphicsItem->setVisible(false);
    }

    _currentHistogram = &_allBandsHistogram;

    // remove children from allBandsHistogram:
    if (_allBandsHistogram.graphicsItem)
    {
        foreach (QGraphicsItem * item, _allBandsHistogram.graphicsItem->childItems())
        {
            _allBandsHistogram.graphicsItem->removeFromGroup(item);
            item->setVisible(false);
        }
    } else {
        _allBandsHistogram.graphicsItem = new QGraphicsItemGroup();
        _histogramScene.addItem(_allBandsHistogram.graphicsItem);
    }


        // draw all histograms:
    QColor dataColors[] = {QColor(255,0,0,81),
                           QColor(0,255,0,81),
                           QColor(0,0,255,81)};
    int indices[] = {r,g,b};
    for (int i=0;i<_histograms.size();i++)
    {
        HistogramItem & h = _histograms[indices[i]];
        drawHistogramGraphicsItem(&h, QPen(dataColors[i], 0.0));
        _allBandsHistogram.graphicsItem->addToGroup(h.graphicsItem);
    }
}






////*************************************************************************

///*!
//    Method to draw histogram in gray or rgb mode :
//*/
//void HistogramView::drawHistogram()
//{

//    // Check if there is only one histogram added in the list:
//    if (_histograms.size() == 1)
//    {
//        _mode = GRAY;
//        _ui->_isAllBands->setEnabled(false);
//    }
//    else if (_histograms.size() == 3)
//    {
//        _mode = RGB;
//        _ui->_isAllBands->setEnabled(true);
//        initializeAllBandsHistogram();
//    }
//    else
//    {
//        SD_TRACE("HistogramView::drawHistogram() : user should add 1 (gray mode) or 3 (rgb mode) histogram datas");
//        return;
//    }

//    if (_ui->_transferFunction->count()==0)
//    {
//        // hide transfer function option
//        _ui->_transferFunction->setVisible(false);
//        _ui->_transferFunctionLabel->setVisible(false);
//    }

//    _ui->_histList->setEnabled(true);
//    _ui->_histList->setCurrentIndex(0);
//    _ui->_isPartial->setEnabled(true);
//    _ui->_isPartial->setChecked(true);
//    _ui->_isComplete->setEnabled(true);

//    _ui->_discreteColors->setEnabled(true);
//    _ui->_revert->setEnabled(true);

//    _ui->_transferFunction->setEnabled(true);
//    _ui->_transferFunction->setCurrentIndex(0);

//    drawHistogram(0);

//}

////*************************************************************************

///*!
//    Method to set transfer function names
//*/
//void HistogramView::setTransferFunctionNames(const QStringList &transferFunctionNames)
//{
//    _ui->_transferFunction->clear();
//    foreach (QString tf, transferFunctionNames)
//    {
//        _ui->_transferFunction->addItem(tf);
//    }

//    // set visible if specified at least one transfer function
//    if (!_ui->_transferFunction->isVisible())
//    {
//        _ui->_transferFunction->setVisible(true);
//        _ui->_transferFunctionLabel->setVisible(true);
//    }
//}

////*************************************************************************

///*!
//    Slot called when update time is timeout and one need send updated color schema
//*/
//void HistogramView::onUpdateTimerTimeout()
//{
//    emit renderConfigurationChanged(_conf);
//}

////*************************************************************************

///*!
//    Slot to handle isComplete radio button clicked
//*/
//void HistogramView::onDisplayCompleteHist(bool checked)
//{
//    if (!_currentHistogram || !checked)
//        return;

//    double newMin, newMax;
//    newMin = _currentHistogram->xmin;
//    newMax = _currentHistogram->xmax;

//    transformAllItems(newMin, newMax);
//}

////*************************************************************************

///*!
//    Slot to handle the action that displays the histogram fitted to slider positions
//*/
//void HistogramView::onDisplayPartialHist(bool checked)
//{
//    if (!_currentHistogram || !checked)
//        return;

//    double newMin, newMax;
//    newMin = _currentHistogram->outputStops.first().first;
//    newMax = _currentHistogram->outputStops.last().first;

//    transformAllItems(newMin, newMax);
//}

////*************************************************************************

///*!
//    Slot to handle histogram list current index changed
//*/
//void HistogramView::onHistListIndexChanged(int i)
//{
//    drawHistogram(i);
//}

////*************************************************************************

///*!
//    Slot to handle histogram list current index changed
//*/
//void HistogramView::onTransferFunctionChanged(QString tf)
//{
//    if (!_currentHistogram || _currentHistogram->transferFunctionName == tf)
//        return;

//    _currentHistogram->transferFunctionName = tf;
//    emit renderConfigurationChanged(_conf);
//}

////*************************************************************************

///*!
//    Slot to handle discrete colors option of the histogram
//*/
//void HistogramView::onDiscreteColorsClicked(bool checked)
//{
//    _colorPalette->setIsDiscrete(checked);
//    _currentHistogram->isDiscrete = _colorPalette->isDiscrete();
//    emit renderConfigurationChanged(_conf);
//}

////*************************************************************************

///*!
//    Slot to handle when all bands option is checked/unchecked
//*/
//void HistogramView::onIsAllBandsClicked(bool checked)
//{
//    if (checked)
//    {
//        _ui->_histList->setEnabled(false);
//        _ui->_discreteColors->setEnabled(false);
//        _ui->_transferFunction->setEnabled(false);
//        drawAllHistograms();
//    }
//    else
//    {
//        _ui->_histList->setEnabled(true);
//        _ui->_discreteColors->setEnabled(true);
//        _ui->_transferFunction->setEnabled(true);

//        // set invisible all histograms:
//        for (int i=0;i<_histograms.size();i++)
//        {
//            _histograms[i].graphicsItem->setVisible(false);
//            _histograms[i].xmin = _allBandsHistogram.xmin;
//            _histograms[i].xmax = _allBandsHistogram.xmax;
//            _allBandsHistogram.graphicsItem->removeFromGroup(_histograms[i].graphicsItem);
//        }
//        // draw
//        drawHistogram(_ui->_histList->currentIndex());
//    }

//}

//*************************************************************************

/*!
    on Show event
*/
void HistogramView::showEvent(QShowEvent * event)
{
    if (event->type() == QEvent::Show)
    {
        _ui->_histogramView->fitInView(_histogramScene.sceneRect());
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
        _ui->_histogramView->fitInView(_histogramScene.sceneRect());
    }
}

////*************************************************************************

///*!
//    Method to transform all graphics items
//*/
//void HistogramView::transformAllItems(double newMin, double newMax)
//{
//    // transform histogram graphics item
//    QTransform tr = _currentHistogram->graphicsItem->transform();
//    updateToPartialMode(_currentHistogram->graphicsItem, _currentHistogram->vxmin, _currentHistogram->vxmax, newMin, newMax);
//    _currentHistogram->graphicsItem->setTransform(tr, true);

//    // transform slider lines
//    foreach (QGraphicsLineItem * line, _sliderLines)
//    {
//        QTransform tr = line->transform();
//        updateToPartialMode(line, _currentHistogram->vxmin, _currentHistogram->vxmax, newMin, newMax);
//        line->setTransform(tr, true);
//    }

//    // transform slider inner positions
//    _colorPalette->setMinMaxRanges(newMin, newMax);

//    // update visual histogram range
//    _currentHistogram->vxmin=newMin;
//    _currentHistogram->vxmax=newMax;
//}

////*************************************************************************

///*!
//    Method to draw the histogram at index
//*/
//void HistogramView::drawHistogram(int index)
//{

//    if (index < 0 || index >=_histograms.size())
//        return;

//    // make invisible current histogram:
//    if (_currentHistogram && _currentHistogram->graphicsItem)
//        _currentHistogram->graphicsItem->setVisible(false);

////    HistogramItem & h = _histograms[index];
//    _currentHistogram = &_histograms[index];

//    drawHistogramGraphicsItem(_currentHistogram, _settings.dataPen);

//    // setup transfer function and isDiscrete coulours :
//    _ui->_discreteColors->setChecked(_currentHistogram->isDiscrete);
//    int i = _ui->_transferFunction->findText(_currentHistogram->transferFunctionName);
//    if (i < 0)
//    {
//        i = 0;
//        _currentHistogram->transferFunctionName=_ui->_transferFunction->itemText(i);
//    }
//    _ui->_transferFunction->setCurrentIndex(i);

//    // draw color palette, sliders, etc in 100% view mode
//    drawColorPalette(_currentHistogram->outputStops, _currentHistogram->vxmin, _currentHistogram->vxmax, _currentHistogram->isDiscrete);

//    // default choice is 95% of histogram to display
//    _ui->_isPartial->setChecked(true);
//    onDisplayPartialHist(true);

//}

////*************************************************************************

///*!
//    Method to draw all histograms
//*/
//void HistogramView::drawAllHistograms()
//{
//    _currentHistogram = &_allBandsHistogram;
//    // draw all histograms:
//    QList<QColor> dataColors = QList<QColor>()
//            << QColor(255,0,0,81)
//            << QColor(0,255,0,81)
//            << QColor(0,0,255,81);
//    for (int i=0;i<_histograms.size();i++)
//    {
//        HistogramItem & h = _histograms[i];
//        drawHistogramGraphicsItem(&h, QPen(dataColors[i], 0.0));

//        // set stops of the band histogram equal to the stops of allBands;
//        copyPositions(_currentHistogram->outputStops,h.outputStops);
//        _allBandsHistogram.graphicsItem->addToGroup(h.graphicsItem);
//    }

//    // draw color palette, sliders, etc in 100% view mode
//    drawColorPalette(_currentHistogram->outputStops, _currentHistogram->vxmin, _currentHistogram->vxmax, false);

//    // default choice is 95% of histogram to display
//    _ui->_isPartial->setChecked(true);
//    onDisplayPartialHist(true);

//}

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
                    (i+1)*1.0/histSize + d,
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

///*!
//    Event filter :
//     - to limit mouse interaction with sliders
//*/
//bool HistogramView::eventFilter(QObject * object, QEvent * event)
//{
//    if (&_histogramScene == object && _histograms.size() > 0)
//    {
//        if (event->type() == QEvent::GraphicsSceneMousePress)
//        {
//            QGraphicsSceneMouseEvent* e = static_cast<QGraphicsSceneMouseEvent*>(event);
//            QGraphicsItem * itemUnderMouse = _histogramScene.itemAt(e->scenePos(), QTransform());

//            if (_colorPalette->itemIsSlider(itemUnderMouse) && _mode == GRAY )
//            { // if clicked on slider :
//                switch (e->button())
//                {
//                case Qt::RightButton:
//                { // open context menu : - remove slider if nb(sliders) > 3
//                    if (_colorPalette->getNbOfSliders() > 3)
//                    {
//                        _addSliderAction.setVisible(false);
//                        _removeSliderAction.setVisible(true);
//                        _revertSliderAction.setVisible(true);
//                        _indexOfActionedSlider = _colorPalette->getSliderIndex(itemUnderMouse);
//                        _menu.popup(e->screenPos());
//                    }
//                    else if (_colorPalette->getNbOfSliders() == 3)
//                    {
//                        _addSliderAction.setVisible(false);
//                        _removeSliderAction.setVisible(false);
//                        _revertSliderAction.setVisible(true);
//                        _indexOfActionedSlider = _colorPalette->getSliderIndex(itemUnderMouse);
//                        _menu.popup(e->screenPos());
//                    }
//                    e->accept();
//                    return true;
//                }
//                }xmin
//            }
//            else if (_colorPalette->itemIsPalette(itemUnderMouse) && _mode == GRAY)
//            { // if clicked on color palette rectangle

//                switch (e->button())
//                {
//                case Qt::RightButton:
//                { // open context menu : - add slider

//                    _removeSliderAction.setVisible(false);
//                    _revertSliderAction.setVisible(false);
//                    _addSliderAction.setVisible(true);
//                    _addSliderAction.setData(
//                                itemUnderMouse->mapFromScene(e->scenePos())
//                                );
//                    _menu.popup(e->screenPos());
//                    e->accept();
//                    return true;
//                }
//                }
//            }

//        }
//        else if (event->type() == QEvent::GraphicsSceneMouseDoubleClick)
//        {
//            QGraphicsSceneMouseEvent* e = static_cast<QGraphicsSceneMouseEvent*>(event);
//            QGraphicsItem * itemUnderMouse = _histogramScene.itemAt(e->scenePos(), QTransform());
//            if (_colorPalette->itemIsSlider(itemUnderMouse) && _mode == GRAY)
//            { // if double-clicked on slider :
//                switch (e->button())
//                {
//                case Qt::LeftButton:
//                { // open color picker
//                    _colorPicker.move(e->screenPos());
//                    _colorPicker.show();
//                    QPoint p = QWidget::mapToGlobal(QPoint(0,0));
//                    if (_colorPicker.x() + _colorPicker.width() > p.x() + this->width())
//                    {
//                        _colorPicker.move(e->screenPos() - QPoint(_colorPicker.width(),0));
//                    }
//                    _indexOfActionedSlider = _colorPalette->getSliderIndex(itemUnderMouse);
//                    e->accept();
//                    return true;
//                }
//                }
//            }
//            else if (_colorPalette->itemIsSliderText(itemUnderMouse))
//            { // if double-clicked on slider-text :
//                switch (e->button())
//                {
//                case Qt::LeftButton:
//                { // open value editor
//                    _indexOfActionedSlider=_colorPalette->getSliderIndex(itemUnderMouse->parentItem());
//                    if (_indexOfActionedSlider < 0)
//                        return false;
//                    double value = _colorPalette->getValue(_indexOfActionedSlider);
//                    if (value < -12344)
//                        return false;

//                    QPoint p = QWidget::mapToGlobal(QPoint(0,0));
//                    int x = e->screenPos().x();
//                    int y = p.y() + _ui->_histogramView->height();
//                    _valueEditor.move(QPoint(x,y));
//                    _valueEditor.setText(QString("%1").arg(value));
//                    _valueEditor.show();
//                    _valueEditor.resize(60,_valueEditor.height());
//                    if (_valueEditor.x() + _valueEditor.width() > p.x() + this->width())
//                    {
//                        _valueEditor.move(QPoint(x - _valueEditor.width(),y));
//                    }

//                    // highlight slider text:
//                    _colorPalette->highlightSliderTextAtIndex(_indexOfActionedSlider);

//                    e->accept();
//                    return true;
//                }
//                }
//            }
//        }
//    }
//    else if (&_valueEditor == object)
//    {
//        if (event->type() == QEvent::KeyPress)
//        { // Hide valueEditor when user presses escape key
//            QKeyEvent * ke = static_cast<QKeyEvent*>(event);
//            if (ke->key() == Qt::Key_Escape && _valueEditor.isVisible())
//            {
//                _valueEditor.hide();
//                _colorPalette->highlightSliderTextAtIndex(_indexOfActionedSlider, false);
//                _indexOfActionedSlider=-1;
//            }
//        }
//        else if (event->type() == QEvent::MouseButtonPress)
//        { // Hide valueEditor when user clicks somewhere else
//            QMouseEvent * me = static_cast<QMouseEvent*>(event);
//            if (!_valueEditor.rect().contains(me->pos()))
//            {
//                _valueEditor.hide();
//                _colorPalette->highlightSliderTextAtIndex(_indexOfActionedSlider, false);
//                _indexOfActionedSlider=-1;
//            }
//        }
//    }
//    return QWidget::eventFilter(object, event);
//}

//*************************************************************************

}
