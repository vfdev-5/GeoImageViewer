
// Qt
#include <QPointF>
#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>


// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "DarkPixelFilterToolPlugin.h"
#include "Core/Global.h"
#include "Core/LayerUtils.h"


namespace Plugins
{

//******************************************************************************
/*!
 * \class DarkPixelFilterToolPlugin
 * \brief is a plugin to segment dark pixel zones. 
 *  Method :
 *  1) Image -> 1.0/Image
 *  2) Convert to 8U
 *  3) Adaptive thresholding
 *  4) Morpho close+dilate
 */
//******************************************************************************

DarkPixelFilterToolPlugin::DarkPixelFilterToolPlugin(QObject * parent) :
    Tools::FilterTool(0, 0, parent),
    _minSize(4)
{
    setObjectName("darkpixels");
    _name=tr("Dark pixels segmentation");
    _description=tr("Filter to segment dark pixel zones of the image");
    _icon = QIcon(":/icons/wand");
}

//******************************************************************************

cv::Mat DarkPixelFilterToolPlugin::processData(const cv::Mat &data)
{
    cv::Mat out;
    if (data.channels() > 1)
    {
        // create gray image
        std::vector<cv::Mat> iChannels(data.channels());
        cv::split(data, &iChannels[0]);

        double a = 1.0/iChannels.size();
        iChannels[0].convertTo(out, CV_32F, a);
        for (int i=1; i<iChannels.size();i++)
        {
            out += a*iChannels[i];
        }
    }
    else
    {
        if (data.depth() == CV_32F)
        {
            data.copyTo(out);
        }
        else
        {
            data.convertTo(out, CV_32F);
        }
    }

    // 1) Image -> 1.0/Image
    cv::divide(1.0,out,out,CV_32F);

    // 2) Convert to 8U
    double minVal, maxVal;
    cv::minMaxLoc(out, &minVal, &maxVal);
    cv::Mat out8U;
    out.convertTo(out8U, CV_8U, 255.0/(maxVal-minVal), -255.0*minVal/(maxVal-minVal));
    out = out8U;


    // 3) Adaptive thresholding
    int winsize = 131;
    cv::Scalar mean = cv::mean(out);
    double c = -mean[0];
    cv::adaptiveThreshold(out, out, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, winsize, c);

    // 4) Morpho close + open
    cv::Mat k1=cv::Mat::ones(5,5,CV_8U);
    cv::Mat k2=cv::Mat::ones(3,3,CV_8U);
    cv::morphologyEx(out, out, cv::MORPH_CLOSE, k1);
    cv::morphologyEx(out, out, cv::MORPH_OPEN, k2);


    // Render to RGBA :
    std::vector<cv::Mat> oChannels(4);
    oChannels[0] = oChannels[1] = oChannels[2] = out;
    oChannels[3] = cv::Mat(data.rows, data.cols, CV_8U, cv::Scalar(255));
    cv::merge(oChannels, out);

    return out;
}

//******************************************************************************

void DarkPixelFilterToolPlugin::onFinalize()
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
