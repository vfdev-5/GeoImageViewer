#ifndef ConvertTo8U_H
#define ConvertTo8U_H

// Qt
#include <QObject>

// Project
#include "Filters/AbstractFilter.h"

namespace Filters
{

//******************************************************************************

class GIV_DLL_EXPORT ConvertTo8U : public AbstractFilter
{
    Q_OBJECT
    Q_PROPERTY_WITH_ACCESSORS(QString, type, getType, setType)
    Q_CLASSINFO("type","possibleValues:real min/max,use mean/std,quantiles,user min/max")

    Q_PROPERTY_WITH_ACCESSORS(double, stdFactor, getStdFactor, setStdFactor)
    Q_CLASSINFO("stdFactor","label:Factor to compute min/max as mean +/- factor * std;minValue:0.001;maxValue:500.0")

    Q_PROPERTY_WITH_ACCESSORS(double, minQuantile, getMinQuantile, setMinQuantile)
    Q_CLASSINFO("minQuantile","label:Minimal quantile;minValue:0.0;maxValue:100.0")

    Q_PROPERTY_WITH_ACCESSORS(double, maxQuantile, getMaxQuantile, setMaxQuantile)
    Q_CLASSINFO("maxQuantile","label:Maximal quantile;minValue:0.0;maxValue:100.0")

    Q_PROPERTY_WITH_ACCESSORS(double, umin, getUserMin, setUserMin)
    Q_CLASSINFO("umin","label:User specified min value;minValue:-1000000000.0;maxValue:1000000000.0")

    Q_PROPERTY_WITH_ACCESSORS(double, umax, getUserMax, setUserMax)
    Q_CLASSINFO("umax","label:User specified max value;minValue:-1000000000.0;maxValue:1000000000.0")



public:
    ConvertTo8U(QObject * parent = 0);
    virtual ~ConvertTo8U() {}

protected:

    virtual cv::Mat filter(const cv::Mat & src) const;

};

//******************************************************************************

}

#endif // ConvertTo8U_H
