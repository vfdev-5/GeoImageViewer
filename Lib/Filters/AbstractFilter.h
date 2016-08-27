#ifndef ABSTRACTFILTER_H
#define ABSTRACTFILTER_H

// Qt
#include <QObject>

// Opencv
#include <opencv2/core/core.hpp>

// Project
#include "Core/LibExport.h"
#include "Core/Global.h"
#include "FiltersManager.h"

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

    Q_PROPERTY_WITH_ACCESSORS(bool, verbose, isVerbose, setVerbose)

    Q_PROPERTY_WITH_ACCESSORS(float, maskByValue, getMaskByValue, setMaskByValue)
    Q_CLASSINFO("maskByValue", "label:Mask from data value (default, -12345 and no mask);minValue:-12345;maxValue:100000")

    // This is needed to access '_errorMessage' attribute
    friend class FilterTask;

public:
    AbstractFilter(QObject * parent = 0);
    virtual ~AbstractFilter() {}

    cv::Mat apply(const cv::Mat & src) const;

    enum {
        Type = 0,
    };

    QString getErrorMessage() const
    { return _errorMessage; }

signals:
    void progressValue(int) const;
    //! Signal sends image copied data and the title of the verbose window. It is receiver responsibility to destroy the data
    void verboseImage(const QString &, cv::Mat *) const;

protected:
    virtual cv::Mat filter(const cv::Mat & src) const = 0;
    mutable QString _errorMessage;

    void verboseDisplayImage(const QString & winname, const cv::Mat & img) const;

};

//******************************************************************************

}

#endif // AbstractFilter_H
