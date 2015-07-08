
// Qt
#include <QPointF>
#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>


// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "FloodThresholdFilterToolPlugin.h"
#include "Core/Global.h"
#include "Core/LayerUtils.h"


namespace Plugins
{

//******************************************************************************
/*!
 * \class FloodThresholdFilterToolPlugin
 * \brief is a plugin of filtering tools that segments image data using flood fill
 *
 */
//******************************************************************************

FloodThresholdFilterToolPlugin::FloodThresholdFilterToolPlugin(QObject * parent) :
    Tools::FilterTool(0, 0, parent),
    _loDiff(50),
    _upDiff(50)
{
    setObjectName("floodtool");
    _name=tr("Flood Threshold");
    _description=tr("Filter to segment image using floodfill and thresholding");
    _icon = QIcon(":/icons/wand");
}

//******************************************************************************

cv::Mat FloodThresholdFilterToolPlugin::processData(const cv::Mat &data)
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

    // Smooth
    cv::blur(in, in, cv::Size(3,3));

    // Flood fill
    cv::Mat processedData(in.rows+2,in.cols+2,CV_8U,cv::Scalar(0));
    cv::Point p(in.cols/2, in.rows/2);
    int flag = 8 | (255 << 8) | cv::FLOODFILL_FIXED_RANGE | cv::FLOODFILL_MASK_ONLY;
    cv::floodFill(in, processedData, p, cv::Scalar(0), 0, cv::Scalar::all(_loDiff), cv::Scalar::all(_upDiff), flag);

    // morpho close:
    cv::morphologyEx(processedData, processedData, cv::MORPH_CLOSE, cv::Mat(3,3,CV_8U,cv::Scalar(1)));

    // return result
    cv::Mat out, temp, processedData8U;
    if (processedData.depth() != CV_8U)
    {
        processedData.convertTo(temp, CV_8U);
    }
    else
    {
        temp = processedData;
    }
    processedData8U = temp(cv::Rect(1,1,temp.cols-2,temp.rows-2));

    std::vector<cv::Mat> oChannels(4);
    oChannels[0] = oChannels[1] = oChannels[2] = processedData8U;
    oChannels[3] = cv::Mat(in.rows, in.cols, CV_8U, cv::Scalar(255));
    cv::merge(oChannels, out);

    return out;
}

//******************************************************************************

void FloodThresholdFilterToolPlugin::onFinalize()
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
