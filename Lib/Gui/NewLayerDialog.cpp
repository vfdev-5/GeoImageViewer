
// Project
#include "NewLayerDialog.h"
#include "ui_NewLayerDialog.h"

namespace Gui
{

//******************************************************************************

NewLayerDialog::NewLayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewLayerDialog)
{
    ui->setupUi(this);
}

//******************************************************************************

NewLayerDialog::~NewLayerDialog()
{
    delete ui;
}

//******************************************************************************

QString NewLayerDialog::getName() const
{
    return ui->_name->text();
}

//******************************************************************************

void NewLayerDialog::setExtent(const QRect &r)
{
    ui->_x->setValue(r.x());
    ui->_y->setValue(r.y());
    ui->_width->setValue(r.width());
    ui->_height->setValue(r.height());
}

//******************************************************************************

QRect NewLayerDialog::getExtent() const
{
    return QRect(ui->_x->value(), ui->_y->value(), ui->_width->value(), ui->_height->value());
}

//******************************************************************************

}
