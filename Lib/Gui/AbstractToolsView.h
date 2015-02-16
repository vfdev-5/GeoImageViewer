#ifndef ABSTRACTTOOLSVIEW_H
#define ABSTRACTTOOLSVIEW_H

// Project
#include "Core/LibExport.h"
#include "Tools/AbstractTool.h"

namespace Gui
{

//******************************************************************************

class GIV_DLL_EXPORT AbstractToolsView : public QWidget
{
    Q_OBJECT
public:
    AbstractToolsView(QWidget * parent = 0) :
        QWidget(parent)
    {
    }

    virtual ~AbstractToolsView() {}
    virtual void addTool(Tools::AbstractTool * tool) = 0;
    virtual void setCurrentTool(const QString & toolName) = 0;

signals:
    void toolChanged(const QString & toolName);

protected:


};

//******************************************************************************

}


#endif // ABSTRACTTOOLSVIEW_H
