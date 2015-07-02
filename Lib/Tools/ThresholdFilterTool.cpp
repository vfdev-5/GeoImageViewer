
// Qt
#include <QPointF>
#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>

// Opencv
#include <opencv2/imgproc/imgproc.hpp>


// Project
#include "ThresholdFilterTool.h"
#include "Core/LayerUtils.h"

namespace Tools
{

//******************************************************************************

ThresholdFilterTool::ThresholdFilterTool(QGraphicsScene *scene, QGraphicsView *view, QObject *parent) :
    FilterTool(scene, view, parent),
    _threshold(2500),
    _inverse(false)
{
    setObjectName("thresholdtool");
    _name=tr("Treshold tool");
    _description=tr("Tool to apply a threshold filtering on selected regions of the layer");
    _icon = QIcon(":/icons/filter");
    _cursor = QCursor(Qt::BlankCursor);

}

//******************************************************************************

cv::Mat ThresholdFilterTool::processData(const cv::Mat &data)
{
    ///
    /// Process only the first band
    ///

    cv::Mat in;
    if (data.channels() > 1)
    {
        std::vector<cv::Mat> iChannels(data.channels());
        cv::split(data, &iChannels[0]);
        in = iChannels[0];
    }
    else
    {
        in = data;
    }

    cv::Mat processedData;
    int type = _inverse ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;
    cv::threshold(in, processedData, _threshold, 255, type);
    cv::morphologyEx(processedData, processedData, cv::MORPH_CLOSE, cv::Mat(3,3,CV_8U,cv::Scalar(1)));

    cv::Mat out, processedData8U;
    if (processedData.depth() != CV_8U)
    {
        processedData.convertTo(processedData8U, CV_8U);
    }
    else
    {
        processedData8U = processedData;
    }
    std::vector<cv::Mat> oChannels(4);
    oChannels[0] = oChannels[1] = oChannels[2] = processedData8U;
    oChannels[3] = cv::Mat(in.rows, in.cols, CV_8U, cv::Scalar(255));
    cv::merge(oChannels, out);

    return out;
}

//******************************************************************************
void ThresholdFilterTool::onFinalize()
{
    //  Do something with data
    QImage im = _drawingsItem->getImage();
    cv::Mat image(im.height(), im.width(), CV_8UC4, im.bits());

    cv::dilate(image, image, cv::Mat());

    std::vector<cv::Mat> iChannels(image.channels());
    cv::split(image, &iChannels[0]);
    QVector<QPolygonF> contours = Core::vectorizeAsPolygons(iChannels[0]);

    if (contours.isEmpty())
    {
        SD_TRACE("Contours are not found");
    }
    else
    {
//        SD_TRACE(QString("Number of contours : %1").arg(contours.size()));
        QGraphicsItemGroup * group = new QGraphicsItemGroup();
        foreach (QPolygonF contour, contours)
        {
            if (contour.last() == contour.first())
            {
                contour.takeLast();
            }
//            SD_TRACE(QString("Number of points : %1").arg(contour.size()));
            if (contour.size() > 2)
            {
                QGraphicsPolygonItem * p = new QGraphicsPolygonItem(contour);
                p->setPen(QPen(QColor(255,0,0,127), 0));
                p->setBrush(QColor(255,0,0,127));
                group->addToGroup(p);
            }
        }

        _scene->addItem(group);
        group->setPos(_drawingsItem->scenePos());
        emit itemCreated(group);

    }
    _scene->removeItem(_drawingsItem);
    delete _drawingsItem;

    //release _drawingsItem pointer
    FilterTool::onFinalize();
}

//******************************************************************************

}
