
// Qt
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTreeWidget>

// Project
#include "SubdatasetDialog.h"

namespace Gui
{

//******************************************************************************

SubdatasetDialog::SubdatasetDialog(const QStringList & subdatasetNames, QWidget *parent) :
    QDialog(parent)
{

    QVBoxLayout * layout = new QVBoxLayout(this);

    _table = new QTreeWidget(this);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setHeaderLabels(QStringList() << tr( "Index" ) << tr( "Subdataset description" ) );
    setWindowTitle( tr( "Select subdataset to add..." ) );

    for( int i=0; i<subdatasetNames.size();i++)
    {
        QStringList fields = QStringList() << QString::number(i) << subdatasetNames[i];
        _table->addTopLevelItem(new QTreeWidgetItem(fields));
    }
    // resize columns
    for ( int i = 0; i < _table->columnCount(); i++ )
    {
        _table->resizeColumnToContents( i );
    }
    _table->setCurrentItem(_table->topLevelItem(0));

    layout->addWidget(_table);

    QDialogButtonBox * buttons = new QDialogButtonBox(this);
    buttons->setOrientation(Qt::Horizontal);
    buttons->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttons);


}

//******************************************************************************

int SubdatasetDialog::getSelectionIndex()
{
    int output=-1;
    if (!_table->selectedItems().isEmpty())
        output = _table->selectedItems()[0]->text(0).toInt();
    return output;
}

//******************************************************************************

}
