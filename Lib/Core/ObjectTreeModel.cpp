
// Qt
#include <QEvent>
#include <QChildEvent>
#include <QMimeData>
#include <QMetaProperty>

// Project
#include "Global.h"
#include "ObjectTreeModel.h"

namespace Core
{

QByteArray encode(const QModelIndexList & indexes)
{
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    foreach (QModelIndex modelIndex, indexes)
    {
        // write model index unique representation chain :
        // before_root_parent.row, ..., parent.parent.row, parent.row, modelIndex.row, -1
        QString indexChain;
        QModelIndex index = modelIndex;
        while(index.isValid())
        {
            indexChain.prepend(QString("%1 ").arg(index.row()));
            index = index.parent();
        }
        indexChain.remove(indexChain.size()-1,1); // remove the last space char
        stream << indexChain;
//        SD_TRACE1("Stream string : %1", indexChain);
    }
    return encoded;
}

QModelIndexList decode(QByteArray & encoded, const QAbstractItemModel * model)
{
    QModelIndexList indices;
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    while (!stream.atEnd())
    {
        QModelIndex index;
        QString indexChain;
        stream >> indexChain;
        QStringList ilist = indexChain.split(" ");
        foreach (QString i, ilist)
        {
            int irow = i.toInt();
            index = model->index(irow, 0, index);
        }
        indices << index;
    }
    return indices;
}

//******************************************************************************
/*!
    \class ObjectTreeModel
    \brief inherits QAbstractItemModel and represents a model for QTreeView
    to display QObject parent/children tree structure.


    Usage:

    // Setup object's parent/children tree
    QObject * invisibleRoot = new QObject(this); // parented to the main structure

    QObject * mainObject = new ClassA(invisibleRoot);
    // ClassA inherits from QObject
    //  and has declared properties : labelA (QString)

    QObject * child1 = new ClassB(mainObject);
    // ClassB inherits from ClassA
    //  and has declared properties : isVisible (Qt::CheckState)
    QObject * child2 = new ClassB(mainObject);

    QObject * child3 = new ClassC(mainObject);
    // ClassC inherits from ClassA
    //  and has declared properties : icon (QImage)
    QObject * child11 = new ClassC(child1);

    QObject * child31 = new ClassD(child3);
    // ClassD inherits from ClassA
    //  and has declared properties : ...
    // ...etc

    // Setup model/view structures
    ObjectTreeModel * model = new ObjectTreeModel(invisbleRoot, this);
    // setup a map between QObject property and a ObjectTreeModel role :
    model->setRole(Qt::DisplayRole, "labelA");
    model->setRole(Qt::EditRole, "labelA");
    model->setRole(Qt::DecorationRole, "icon");
    model->setRole(Qt::CheckStateRole, "isVisible");
    // ...

    QTreeView * view = new QTreeView(this);
    view->setModel(model);


 */

//******************************************************************************

/*!
 * \brief ObjectTreeModel::ObjectTreeModel
 * \param root is the invisible root of the QObject parent/children tree structure
 * \param parent
 */
ObjectTreeModel::ObjectTreeModel(QObject * root, QObject *parent) :
    QAbstractItemModel(parent),
    _root(root)
{

    // install event filter on the whole tree of children
    QObjectList stack = QObjectList();
    stack << _root;
    while(!stack.isEmpty())
    {
        QObject * child = stack.takeLast();
        child->installEventFilter(this);
        stack << child->children();
    }

}

//******************************************************************************

void ObjectTreeModel::setRole(int role, const QString &propertyName)
{
    _rolePropertyMap.insert(role, propertyName);
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
        // !!! Need to order
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

    // !!! Need to order
    int row = parentObject2->children().indexOf(parentObject);

    return createIndex(row , 0, parentObject );
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

#define IfDefaultDisplayRole()\
if (role == Qt::DisplayRole) \
{                               \
    QString o = object->objectName(); \
    if (o.isEmpty()) \
        o = object->metaObject()->className(); \
    return o; \
}

QVariant ObjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();


    QObject * object = static_cast<QObject*>(index.internalPointer());
    if (index.column() == 0 && object)
    {
        QString propertyName = _rolePropertyMap.value(role, "");
        if (!propertyName.isEmpty())
        {
            const QMetaObject * metaObject = object->metaObject();
            int propertyIndex = metaObject->indexOfProperty(propertyName.toLatin1().data());
            if (propertyIndex < 0)
            {
                IfDefaultDisplayRole();
//                SD_TRACE1("ObjectTreeModel::data : property %1 is not found", propertyName);
                return QVariant();
            }
            QMetaProperty property = metaObject->property(propertyIndex);
            if (!property.isValid())
            {
                IfDefaultDisplayRole();
//                SD_TRACE("ObjectTreeModel::data : meta property is invalid");
                return QVariant();
            }

            QVariant out = property.read(object);
            // Special process for boolean type
            if (out.isValid() && out.type() == QVariant::Bool)
            {
                bool b = out.toBool();
                out = QVariant(b ? Qt::Checked : Qt::Unchecked);
            }
            return out;
        }
        else
        {
            IfDefaultDisplayRole();
        }
    }
    return QVariant();
}

//******************************************************************************

#define IfDefaultEditRole()\
if (role == Qt::EditRole) \
{                               \
    QString o = value.toString(); \
    if (!o.isEmpty()) \
    {                   \
        object->setObjectName(o); \
        emit dataChanged(index, index); \
    }                       \
    return true; \
}

bool ObjectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    QObject * object = static_cast<QObject*>(index.internalPointer());
    if (index.column() == 0 && object)
    {
        QString propertyName = _rolePropertyMap.value(role, "");
        if (!propertyName.isEmpty())
        {
            const QMetaObject * metaObject = object->metaObject();
            int propertyIndex = metaObject->indexOfProperty(propertyName.toLatin1().data());
            if (propertyIndex < 0)
            {
                IfDefaultEditRole();
//                SD_TRACE1("ObjectTreeModel::data : property %1 is not found", propertyName);
                return false;
            }
            QMetaProperty property = metaObject->property(propertyIndex);
            if (!property.isValid())
            {
                IfDefaultEditRole();
////                SD_TRACE("ObjectTreeModel::data : meta property is invalid");
                return false;
            }

            if (!property.isWritable())
            {
                SD_TRACE("ObjectTreeModel::setData : meta property is not writable");
                return false;
            }

            // Special process for boolean type
            QVariant out = value;
            if (role == Qt::CheckStateRole)
            {
                Qt::CheckState s = (Qt::CheckState) out.value<int>();
                out = QVariant(s == Qt::Checked ? true : false);
            }

            if (!property.write(object, out))
            {
                SD_TRACE("ObjectTreeModel::setData : meta property is not writable");
                return false;
            }

            emit dataChanged(index, index);
            return true;
        }
        else
        {
            IfDefaultEditRole();
        }
    }
    return false;
}

//******************************************************************************

Qt::ItemFlags ObjectTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    // if item is not root :
    Qt::ItemFlags flags = Qt::ItemIsSelectable |
            Qt::ItemIsEditable |
            Qt::ItemIsEnabled |
            Qt::ItemIsUserCheckable;
    if (_root != index.internalPointer())
    {
        flags |= Qt::ItemIsDragEnabled |
                Qt::ItemIsDropEnabled;
    }
    return flags;
}

//******************************************************************************

Qt::DropActions ObjectTreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

//******************************************************************************

//bool ObjectTreeModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//    SD_TRACE2("- insertRows : row=%1, count=%2", row, count);
//    SD_TRACE1("-- insertRows : parent.row=%1", parent.row());
//    return QAbstractItemModel::insertRows(row, count, parent);
//}

//******************************************************************************

//bool ObjectTreeModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
//{
//    SD_TRACE("moveRows");
//    return false;
//}

//******************************************************************************

QStringList ObjectTreeModel::mimeTypes() const
{
    QStringList types = QAbstractItemModel::mimeTypes();
    types << "text/plain";
    return types;
}

//******************************************************************************

//bool ObjectTreeModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//    if (parent.isValid())
//    {
//        for (int i=row;i<row+count;i++)
//        {
//            for (int j=0;j<columnCount(parent);j++)
//            {
//                QModelIndex child = parent.child(i,j);
//                QObject * object = static_cast<QObject*>(child.internalPointer());
//                if (!object)
//                {
//                    SD_TRACE2("ObjectTreeModel::removeRows : object is null for the model index : ", i, j);
//                    continue;
//                }
//                object->setParent(0);
//                delete object;
//            }
//        }
//    }
//    else
//    {
//    }
//    SD_TRACE2("- removeRows : row=%1, count=%2", row, count);
//    SD_TRACE1("-- removeRows : parent.row=%1", parent.row());
//    return QAbstractItemModel::removeRows(row, count, parent);
//}

//******************************************************************************

QMimeData *ObjectTreeModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() <= 0)
        return 0;
//    QStringList types = mimeTypes();
//    if (types.isEmpty())
//        return 0;
    QMimeData *data = new QMimeData();
    QString format = "text/plain";
    QByteArray encoded = encode(indexes);
    data->setData(format, encoded);
    return data;
}

//******************************************************************************

bool ObjectTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
//    SD_TRACE("dropMimeData");
    if (!QAbstractItemModel::canDropMimeData(data, action, row, column, parent))
    {
        return false;
    }

    if (data && action == Qt::MoveAction)
    {
//        SD_TRACE2("- dropMimeData : dst row=%1, column=%2", row, column);
//        SD_TRACE2("-- dropMimeData : parent row=%1, column=%2", parent.row(), parent.column());
        int beginRow;
        if (row > -1)
        {
            beginRow = row;
        }
        else if (parent.isValid())
        {
            beginRow = parent.row();
        }
        else
        {
            beginRow = rowCount(QModelIndex());
        }

        QObject * parentObject = static_cast<QObject*>(parent.internalPointer());
        SD_TRACE_PTR("--- dropMimeData : dstParentObject", parentObject);
        if (parentObject)
        {
            QByteArray encodedData = data->data("text/plain");
            QModelIndexList indices = decode(encodedData, this);
            foreach (QModelIndex index, indices)
            {
                QObject * object = static_cast<QObject*>(index.internalPointer());
                SD_TRACE_PTR("--- dropMimeData : object", object);
                if (object)
                {
                    // To explicitly change order of children
                    if (object->parent() == parentObject)
                    {
                        object->setParent(0);
                    }
                    object->setParent(parentObject);
                }
            }
        }
    }
    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
//    return false;
}

//******************************************************************************

bool ObjectTreeModel::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::ChildAdded)
    {
        QChildEvent * childEvent = static_cast<QChildEvent*>(event);
        QObject * child = childEvent->child();
        child->installEventFilter(this);

//        SD_TRACE_PTR("- Child : ", child);
//        SD_TRACE_PTR("- Child added to object", object);

        // find parent's model index :
        int row = object->children().indexOf(child);
        QModelIndex parent = this->parent(createIndex(row, 0, child));
//        SD_TRACE_PTR("-- Child added : parent == object", parent.internalPointer());
//        SD_TRACE2("-- Child added : parent.row=%1, parent.column=%2", parent.row(), parent.column());
//        SD_TRACE1("-- new object position=%1", row);
        beginInsertRows(parent, row, row);
        endInsertRows();
    }
    else if (event->type() == QEvent::ChildRemoved)
    {
        QChildEvent * childEvent = static_cast<QChildEvent*>(event);
        QObject * child = childEvent->child();
        child->removeEventFilter(this);

//        SD_TRACE_PTR("- Child : ", child);
//        SD_TRACE_PTR("- Child removed from object", object);
        QObject * grandparent = object->parent();
        int row = grandparent ? grandparent->children().indexOf(object) : 0;
        QModelIndex parent = createIndex(row, 0, object);

        // Inform that all children have been removed <= because we can not indicate from which row a child has been removed
        int count = object->children().count();
        if (count == 0) count = 1;
        beginRemoveRows(parent, 0, count-1);
        endRemoveRows();
    }

    return QAbstractItemModel::eventFilter(object, event);
}

//******************************************************************************

}
