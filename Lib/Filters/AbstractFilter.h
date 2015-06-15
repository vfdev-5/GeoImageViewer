#ifndef ABSTRACTFILTER_H
#define ABSTRACTFILTER_H

// Qt
#include <QObject>

// Opencv
#include <opencv2/core/core.hpp>

// Project
#include "Core/LibExport.h"
#include "Core/Global.h"

namespace Filters
{

//******************************************************************************

class GIV_DLL_EXPORT AbstractFilter : public QObject
{
    Q_OBJECT
    PROPERTY_GETACCESSOR(int, filterType, getType)
    Q_PROPERTY_WITH_GETACCESSOR(QString, name, getName)
    Q_PROPERTY_WITH_GETACCESSOR(QString, description, getDescription)

    PROPERTY_GETACCESSOR(QString, errorMessage, getErrorMessage)
    PROPERTY_ACCESSORS(float, noDataValue, getNoDataValue, setNoDataValue)

public:
    AbstractFilter(QObject * parent = 0);
    virtual ~AbstractFilter() {}

    cv::Mat apply(const cv::Mat & src) const;

    enum {
        Type = 0,
    };

protected:
    virtual cv::Mat filter(const cv::Mat & src) const = 0;

};

//******************************************************************************

}

#endif // AbstractFilter_H
