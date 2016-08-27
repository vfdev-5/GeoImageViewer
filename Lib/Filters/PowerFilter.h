#ifndef POWERFILTER_H
#define POWERFILTER_H

// Qt
#include <QObject>

// Project
#include "Filters/AbstractFilter.h"

namespace Filters
{

//******************************************************************************

class GIV_DLL_EXPORT PowerFilter : public AbstractFilter
{
    Q_OBJECT

    Q_PROPERTY_WITH_ACCESSORS(double, power, getPower, setPower)


public:
    PowerFilter(QObject *parent = 0);


protected:
    virtual cv::Mat filter(const cv::Mat & src) const;

};

//******************************************************************************

}

#endif // POWERFILTER_H
