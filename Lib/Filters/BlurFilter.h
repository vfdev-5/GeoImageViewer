#ifndef BLURFILTER_H
#define BLURFILTER_H

// Qt
#include <QObject>

// Project
#include "Filters/AbstractFilter.h"

namespace Filters
{

//******************************************************************************

class GIV_DLL_EXPORT BlurFilter : public AbstractFilter
{
    Q_OBJECT

    Q_PROPERTY_WITH_ACCESSORS(int, size, getSize, setSize)

    Q_CLASSINFO("size","minValue:1;maxValue:50")

public:
    BlurFilter(QObject * parent = 0);
    virtual ~BlurFilter() {}

};

//******************************************************************************

}

#endif // AbstractFilter_H
