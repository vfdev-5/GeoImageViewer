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
    PROPERTY_ACCESSORS(float, noDataValue, getNoDataValue, setNoDataValue)

public:
    AbstractFilter(QObject * parent = 0);
    virtual ~AbstractFilter() {}

    cv::Mat apply(const cv::Mat & src) const;

    enum {
        Type = 0,
    };

    QString getErrorMessage() const
    { return _errorMessage; }


protected:
    virtual cv::Mat filter(const cv::Mat & src) const = 0;
    mutable QString _errorMessage;

};

//******************************************************************************

}

#endif // AbstractFilter_H
