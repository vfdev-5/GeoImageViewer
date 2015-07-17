#ifndef OBJECTTREEMODEL_H
#define OBJECTTREEMODEL_H

// Qt
#include <QAbstractItemModel>

// Project
#include "LibExport.h"

namespace Core
{

//******************************************************************************

class GIV_DLL_EXPORT ObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ObjectTreeModel(QObject * root, QObject *parent = 0);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

signals:

public slots:

protected:
    QObject * _root;

};

//******************************************************************************

}

#endif // OBJECTTREEMODEL_H
