
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

cv::Mat AbstractFilter::apply(const cv::Mat &src) const
{
    // Catch Opencv exceptions:
    try
    {
        return filter(src);
    }
    catch (const cv::Exception & e)
    {
        SD_TRACE(QString("OpenCV Error in \'%1\' :\n %2")
               .arg(getName())
               .arg(e.msg.c_str()));
        return cv::Mat();
    }
}

//******************************************************************************

}
