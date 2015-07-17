
// Project
#include "ObjectTreeModel.h"

namespace Core
{

//******************************************************************************
/*!
    \class ObjectTreeModel
    \brief

 */

//******************************************************************************

ObjectTreeModel::ObjectTreeModel(QObject * root, QObject *parent) :
    QAbstractItemModel(parent),
    _root(root)
{

}

//******************************************************************************

QModelIndex ObjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    QObject * parentObject=0;
    if (!parent.isValid())
    {
        parentObject = _root;
    }
    else
    {
        parentObject = static_cast<QObject*>(parent.internalPointer());
    }

    if (parentObject && row < parentObject->children().count())
    {
        return createIndex(row, column, parentObject->children().at(row));
    }
    return QModelIndex();
}

//******************************************************************************

QModelIndex ObjectTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
    {
        return QModelIndex();
    }
    QObject * indexObject = static_cast<QObject*>(child.internalPointer());
    QObject * parentObject = indexObject->parent();
    QObject * parentObject2 = parentObject->parent();
    if (!parentObject ||
            parentObject == _root ||
            !parentObject2)
    {
        return QModelIndex();
    }
    return createIndex( parentObject2->children().indexOf(parentObject), 0, parentObject );
}

//******************************************************************************

int ObjectTreeModel::rowCount(const QModelIndex &parent) const
{
    QObject * object=0;
    if (!parent.isValid())
    {
        object = _root;
    }
    else
    {
        object = static_cast<QObject*>(parent.internalPointer());
    }
    return object ? object->children().count() : -1;
}

//******************************************************************************

int ObjectTreeModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

//******************************************************************************

QVariant ObjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        QObject * object = static_cast<QObject*>(index.internalPointer());
        if (index.column() == 0 && object)
        {
            QString o = object->objectName();
            if (o.isEmpty())
                o = object->metaObject()->className();
            return o;
        }
    }
    return QVariant();
}

//******************************************************************************

}
