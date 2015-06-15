#ifndef NEWLAYERDIALOG_H
#define NEWLAYERDIALOG_H


// Qt
#include <QDialog>

// Project
#include "Core/LibExport.h"

namespace Ui {
class NewLayerDialog;
}

namespace Gui
{

//******************************************************************************

class NewLayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewLayerDialog(QWidget *parent = 0);
    ~NewLayerDialog();

    void setExtent(const QRect & r);

    QString getName() const;
    QRect getExtent() const;

private:
    Ui::NewLayerDialog *ui;
};

//******************************************************************************

}

#endif // NEWLAYERDIALOG_H
