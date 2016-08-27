#ifndef DifferentialFilter_H
#define DifferentialFilter_H

// Qt
#include <QObject>

// Project
#include "Filters/AbstractFilter.h"

namespace Filters
{

//******************************************************************************

class GIV_DLL_EXPORT DifferentialFilter : public AbstractFilter
{
    Q_OBJECT

    Q_PROPERTY_WITH_ACCESSORS(QString, type, getType, setType)
    Q_CLASSINFO("type","possibleValues:sobel,scharr,laplacian")

    Q_PROPERTY_WITH_ACCESSORS(int, size, getSize, setSize)
    Q_CLASSINFO("size","minValue:1;maxValue:100")


public:
    DifferentialFilter(QObject * parent = 0);
    virtual ~DifferentialFilter() {}

protected:

    virtual cv::Mat filter(const cv::Mat & src) const;

};

//******************************************************************************

}

#endif // DifferentialFilter_H
