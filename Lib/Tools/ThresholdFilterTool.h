#ifndef THRESHOLDFILTERTOOL_H
#define THRESHOLDFILTERTOOL_H

// Qt
#include <QObject>
#include <QImage>

// Opencv
#include <opencv2/core/core.hpp>

// Project
#include "AbstractTool.h"
#include "FilterTool.h"

class QGraphicsItem;

namespace Tools
{

//******************************************************************************

class GIV_DLL_EXPORT ThresholdFilterTool : public FilterTool
{
    Q_OBJECT
    Q_PROPERTY_WITH_ACCESSORS(int, threshold, getThreshold, setThreshold)
    Q_CLASSINFO("threshold","minValue:0;maxValue:10000")

    Q_PROPERTY_WITH_ACCESSORS(bool, inverse, inverse, setInverse)

public:
    explicit ThresholdFilterTool(QGraphicsScene* scene, QGraphicsView * view, QObject *parent = 0);

protected slots:
    virtual void onFinalize();

protected:
    virtual cv::Mat processData(const cv::Mat & data);

};

//******************************************************************************

}

#endif // THRESHOLDFILTERTOOL_H
