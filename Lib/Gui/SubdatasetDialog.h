#ifndef SUBDATASETDIALOG_H
#define SUBDATASETDIALOG_H

// Qt
#include <QDialog>

class QTreeWidget;

namespace Gui
{

//******************************************************************************

class SubdatasetDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SubdatasetDialog(const QStringList & subdatasetNames, QWidget *parent = 0);

    int getSelectionIndex();

protected:
    QTreeWidget * _table;

};

//******************************************************************************

}

#endif // SUBDATASETDIALOG_H
