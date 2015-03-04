
// Qt
#include <QVBoxLayout>
#include <QDialogButtonBox>

// Project
#include "DefaultFilterDialog.h"
#include "PropertyEditor.h"
#include "Filters/AbstractFilter.h"

namespace Gui
{

//******************************************************************************

DefaultFilterDialog::DefaultFilterDialog(const QString &title, QWidget *parent) :
    QDialog(parent),
    _editor(new PropertyEditor())
{
    setWindowTitle(title);
    setLayout(new QVBoxLayout());
    layout()->addWidget(_editor);

    _editor->setNameFilter(QStringList() << "objectName");

    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                                      | QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    layout()->addWidget(buttons);

}

//******************************************************************************

void DefaultFilterDialog::setFilter(Filters::AbstractFilter * filter)
{
    _editor->setup(filter);
}

//******************************************************************************

}
