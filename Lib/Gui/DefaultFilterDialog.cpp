
// Qt
#include <QVBoxLayout>
#include <QPushButton>

// Project
#include "DefaultFilterDialog.h"
#include "PropertyEditor.h"
#include "Filters/AbstractFilter.h"

namespace Gui
{

//******************************************************************************

BaseFilterDialog::BaseFilterDialog(const QString &title, QWidget *parent) :
    QWidget(parent)
{
    setWindowTitle(title);
}

//******************************************************************************

DefaultFilterDialog::DefaultFilterDialog(const QString &title, QWidget *parent) :
    BaseFilterDialog(title, parent),
    _editor(new PropertyEditor())
{
    setLayout(new QVBoxLayout());
    layout()->addWidget(_editor);

    _editor->setPropertyUnfilter(QStringList() << "objectName");

    QPushButton * apply = new QPushButton(tr("Apply"));
    layout()->addWidget(apply);

    connect(apply, &QPushButton::clicked, this, &DefaultFilterDialog::applyFilter);
}

//******************************************************************************

void DefaultFilterDialog::setFilter(Filters::AbstractFilter * filter)
{
    _editor->setup(filter);
}

//******************************************************************************

}
