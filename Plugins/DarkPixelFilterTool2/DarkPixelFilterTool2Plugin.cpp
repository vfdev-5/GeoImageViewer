
// Qt
#include <QPointF>
#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>


// Opencv
#include <opencv2/imgproc/imgproc.hpp>

// Project
#include "DarkPixelFilterTool2Plugin.h"
#include "Core/Global.h"
#include "Core/LayerUtils.h"


namespace Plugins
{

//******************************************************************************
/*!
 * \class DarkPixelFilterTool2Plugin
 * \brief is a plugin to segment dark pixel zones. 
 *  Method :
 *  1) Gaussian blur of size minSize
 */
//******************************************************************************

DarkPixelFilterTool2Plugin::DarkPixelFilterTool2Plugin(QObject * parent) :
    Tools::FilterTool(0, 0, parent),
    _minSize(15),
    _param(0.0)
{
    setObjectName("darkpixels2");
    _name=tr("Dark pixels segmentation 2");
    _description=tr("Filter to segment dark pixel zones of the image.");
    _icon = QIcon(":/icons/wand");
}

//******************************************************************************

cv::Mat DarkPixelFilterTool2Plugin::processData(const cv::Mat &data)
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


    // 1) Gaussian blur
    if (_minSize > 1)
    {
        _minSize = (_minSize % 2 == 0) ? _minSize+1: _minSize;
        cv::GaussianBlur(out, out, cv::Size(_minSize, _minSize), 0);
    }

    // 2) Threshold inverse
    // Compute threshold value as mean*0.5
    cv::Scalar mn = cv::mean(out);
    double thresholdValue = 0.75*mn[0] + _param;
//    SD_TRACE1("Threshold value = %1", thresholdValue);
    cv::threshold(out, out, thresholdValue, 255, CV_THRESH_BINARY_INV);

    // 4) Morpho close
    cv::Mat k1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(_minSize/2, _minSize/2));
    cv::morphologyEx(out, out, cv::MORPH_CLOSE, k1);

    // Render to RGBA :
    cv::Mat t1,t2;
    out.convertTo(t1, CV_8U, 1.0/255.0);
    t2 = cv::Mat(data.rows, data.cols, CV_8U, cv::Scalar(255));
    cv::Mat t3[] = {_color.blue()*t1, _color.green()*t1, _color.red()*t1, t2};
    cv::merge(t3, 4, out);

    return out;
}

//******************************************************************************

void DarkPixelFilterTool2Plugin::onFinalize()
{
    //  Do something with data

    QImage im = _drawingsItem->getImage();
    cv::Mat image(im.height(), im.width(), CV_8UC4, im.bits());

    cv::dilate(image, image, cv::Mat());
    cv::Mat gray;
    cv::cvtColor(image, gray, CV_BGRA2GRAY);
    QVector<QPolygonF> contours = Core::vectorizeAsPolygons(gray);

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
