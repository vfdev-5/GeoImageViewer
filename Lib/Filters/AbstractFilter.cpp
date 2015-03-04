
// Project
#include "AbstractFilter.h"

namespace Filters
{

//******************************************************************************
/*!
  \class AbstractFilter
  \brief Abstract class represent application filters

  */

//******************************************************************************

AbstractFilter::AbstractFilter(QObject *parent) :
    QObject(parent),
    _filterType(Type)
{

}

//******************************************************************************

}
