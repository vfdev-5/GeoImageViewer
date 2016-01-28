#ifndef EditableFilterDialog_H
#define EditableFilterDialog_H

// Qt
#include <QWidget>

// Project
#include "Core/LibExport.h"
#include "ui_EditableFilterDialog.h"


namespace Filters
{
class EditableFilter;
}

namespace Gui
{

//******************************************************************************

class BuildConfigDialog;
class BuildErrorWidget;
class Highlighter;

class GIV_DLL_EXPORT EditableFilterDialog : public QWidget
{
    Q_OBJECT

    public:
        explicit EditableFilterDialog(Filters::EditableFilter *f, QWidget *parent = 0);
    ~EditableFilterDialog();

public slots:
    void on__configure_clicked();
    void on__apply_clicked();    

protected:
    virtual void closeEvent(QCloseEvent *);

protected slots:
    void onBadConfiguration();
    void onWorkFinished(bool ok);
    void onBuildError(const QString &);

private:

    void setupEditor();
    void setUiEnabled(bool v);    
    void configure();

    Ui_EditableFilterDialog *ui;
    BuildConfigDialog * _configDialog;
    BuildErrorWidget * _errorWidget;

    Highlighter * _highlighter;

    Filters::EditableFilter * _model;

};

//******************************************************************************

}

#endif // EditableFilterDialog_H
