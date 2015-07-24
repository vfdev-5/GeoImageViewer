#ifndef OBJECTTREEMODEL_H
#define OBJECTTREEMODEL_H

// Qt
#include <QAbstractItemModel>
#include <QHash>

// Project
#include "LibExport.h"

namespace Core
{

//******************************************************************************

class GIV_DLL_EXPORT ObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:

//    enum ItemRole {
//        Ordering = Qt::,
//    };

    explicit ObjectTreeModel(QObject * root, QObject *parent = 0);

    void setRole(int, const QString & propertyName);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual Qt::DropActions supportedDropActions() const;
//    virtual bool insertRows(int row, int count, const QModelIndex &parent);
//    virtual bool removeRows(int row, int count, const QModelIndex &parent);
//    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);
    virtual QStringList mimeTypes() const;
    virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

signals:

public slots:

protected:
    bool eventFilter(QObject *, QEvent *);
    QObject * _root;

    QHash<int, QString> _rolePropertyMap;
    QList<double> _ordering;

};

//******************************************************************************

}

#endif // OBJECTTREEMODEL_H
