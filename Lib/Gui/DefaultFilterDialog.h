#ifndef DEFAULTFILTERDIALOG_H
#define DEFAULTFILTERDIALOG_H


// Qt
#include <QDialog>

// Project
#include "Core/LibExport.h"
#include "Core/Global.h"

namespace Filters
{
class AbstractFilter;
}


namespace Gui
{

class PropertyEditor;

//******************************************************************************

class GIV_DLL_EXPORT BaseFilterDialog : public QWidget
{
    Q_OBJECT
    PROPERTY_GETACCESSOR(QString, layerName, getLayerName)

public:

    explicit BaseFilterDialog(const QString & title, QWidget *parent = 0);
    virtual void setFilter(Filters::AbstractFilter * filter) {}

    virtual void setLayerName(const QString & name)
    { _layerName = name; }

signals:
    void applyFilter();

};



class GIV_DLL_EXPORT DefaultFilterDialog : public BaseFilterDialog
{
    Q_OBJECT
public:
    explicit DefaultFilterDialog(const QString & title, QWidget *parent = 0);
    virtual void setFilter(Filters::AbstractFilter * filter);

protected:
    PropertyEditor * _editor;

};

//******************************************************************************

}

#endif // DEFAULTFILTERDIALOG_H
