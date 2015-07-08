
// Project
#include "AbstractFilter.h"
#include "Core/LayerUtils.h"

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
/*!
  Method to apply a filter on data.

*/
cv::Mat AbstractFilter::apply(const cv::Mat &src) const
{
    // Catch Opencv exceptions:
    try
    {
        // Rewrite noDataValue to zero :
        // Mask is a matrix of nb of channels, depth of src, contains 1.0 values where src data exists
        // and 0.0 values where src data = noDataValue
        cv::Mat mask, unmask;
        mask = Core::computeMask(src, _noDataValue, &unmask);


        cv::Mat srcToProcess = src.mul(mask);

        // Apply filtering:
        cv::Mat res = filter(srcToProcess);

        if (res.empty())
            return res;

        // !!! MAYBE IT IS OK
//        if (res.depth() != src.depth())
//        {
//            SD_TRACE("AbstractFilter::apply : result depth is different from the source depth")
//            return cv::Mat();
//        }
//        if (res.channels() != src.channels())
//        {
//            SD_TRACE("AbstractFilter::apply : result nb of channels is different from the source nb of channels. Return processed matrix without noDataValues");
//            return res;
//        }

        // Write noDataValue:
        unmask = unmask.mul(_noDataValue);
        if (res.type() == unmask.type())
        {
            res = unmask + res;
        }
        else
        {
            cv::Mat m;
            unmask.convertTo(m, res.depth());
            res = m + res;
        }

        return res;
    }
    catch (const cv::Exception & e)
    {
//        SD_TRACE(QString("OpenCV Error in \'%1\' :\n %2")
//               .arg(getName())
//               .arg(e.msg.c_str()));
        _errorMessage = tr("OpenCV Error in \'%1\' :\n %2")
                .arg(getName())
                .arg(e.msg.c_str());
        return cv::Mat();
    }
}

//******************************************************************************

}
