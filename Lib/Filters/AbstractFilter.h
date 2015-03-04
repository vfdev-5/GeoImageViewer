#ifndef ABSTRACTFILTER_H
#define ABSTRACTFILTER_H

// Qt
#include <QObject>

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

public:
    AbstractFilter(QObject * parent = 0);
    virtual ~AbstractFilter() {}

    enum {
        Type = 0,
    };

};

//******************************************************************************

}

#endif // AbstractFilter_H
