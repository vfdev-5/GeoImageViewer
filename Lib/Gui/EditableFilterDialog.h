#ifndef EditableFilterDialog_H
#define EditableFilterDialog_H

// Qt
#include <QWidget>

// Project
#include "DefaultFilterDialog.h"
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

class GIV_DLL_EXPORT EditableFilterDialog : public BaseFilterDialog
{
    Q_OBJECT

public:
    explicit EditableFilterDialog(Filters::EditableFilter *f, QWidget *parent = 0);
    ~EditableFilterDialog();

    virtual void setLayerName(const QString & layerName)
    { ui->_layer->setText(layerName); BaseFilterDialog::setLayerName(layerName); }

public slots:
    void on__configure_clicked();
    void on__apply_clicked();
    void on__refresh_clicked();

protected:
    virtual void closeEvent(QCloseEvent *event);

protected slots:
    void onBadConfiguration();
    void onWorkFinished(bool ok);
    void onBuildError(const QString &);

private:

    void refreshCode();

    void setupEditor();
    void setUiEnabled(bool v);
    void configure();

    void getConfigFromSettings();
    void setConfigToSettings();
    Ui_EditableFilterDialog *ui;
    BuildConfigDialog * _configDialog;
    BuildErrorWidget * _errorWidget;

    Highlighter * _highlighter;

    Filters::EditableFilter * _model;

};

//******************************************************************************

}

#endif // EditableFilterDialog_H
