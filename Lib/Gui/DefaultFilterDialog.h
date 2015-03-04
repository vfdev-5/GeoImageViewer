#ifndef DEFAULTFILTERDIALOG_H
#define DEFAULTFILTERDIALOG_H


// Qt
#include <QDialog>

// Project
#include "Core/LibExport.h"

namespace Filters
{
class AbstractFilter;
}


namespace Gui
{

class PropertyEditor;

//******************************************************************************

class GIV_DLL_EXPORT DefaultFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DefaultFilterDialog(const QString & title, QWidget *parent = 0);
    void setFilter(Filters::AbstractFilter * filter);

signals:

public slots:

protected:

    PropertyEditor * _editor;

};

//******************************************************************************

}

#endif // DEFAULTFILTERDIALOG_H
