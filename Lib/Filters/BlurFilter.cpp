
// Project
#include "BlurFilter.h"


namespace Filters
{

//******************************************************************************
/*!
  \class BlurFilter
  \brief Blur filter implementation

  */

//******************************************************************************

BlurFilter::BlurFilter(QObject *parent) :
    AbstractFilter(parent)
{
    _name = tr("Blur filter");
    _description = tr("Simple blur filter");
}

//******************************************************************************

}
