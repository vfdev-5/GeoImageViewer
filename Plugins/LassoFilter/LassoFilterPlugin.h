#ifndef LASSOFILTERPLUGIN_H
#define LASSOFILTERPLUGIN_H

// Qt
#include <QObject>

// Project
#include "Lasso.h"
#include "Filters/AbstractFilter.h"

namespace Plugins
{

//******************************************************************************

class GIV_PLUGIN_EXPORT LassoFilterPlugin : public Filters::AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "AbstractFilter")

public:

    LassoFilterPlugin(QObject * parent = 0) :
        Filters::AbstractFilter(parent)
    {
        _name=tr("Lasso Filter");
        _description=tr("Lasso filter to select a region");
    }

protected:

    virtual cv::Mat filter(const cv::Mat & ) const;

};

//******************************************************************************

}

#endif // LASSOPLUGIN_H
