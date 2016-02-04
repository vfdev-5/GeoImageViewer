
// STD
#include <algorithm>

// Qt
#include <QCoreApplication>
#include <QEvent>
#include <QChildEvent>
#include <QMimeData>
#include <QMetaProperty>

// Project
#include "Global.h"
#include "ObjectTreeModel.h"
//#include "GeoShapeLayer.h" // !!! REMOVE

namespace Core
{

//******************************************************************************

struct Compare : public std::binary_function<QObject*, QObject*, bool>
{
    enum type {Less, Greater};
    Compare(const char * propertyName, type op) :
        _propertyName(propertyName)
    {
        switch (op) {
        case Less: _func = &Compare::lOp; break;
        case Greater: _func = &Compare::gOp; break;
        default: _func = &Compare::lOp;
        }
    }
    bool operator()(const QObject * __x, const QObject* __y) const
    {
        QVariant __v1 = __x->property(_propertyName);
        QVariant __v2 = __y->property(_propertyName);
        Q_ASSERT_X(__v1.isValid() && __v2.isValid(),
                   "In bool operator()(const QObject * __x, const QObject* __y) of struct less<QObject*>",
                   "Objects have no property");
        return (this->*_func)(__v1,__v2);
    }
private:
    bool lOp(const QVariant & v, const QVariant & w) const
    { return v < w; }
    bool gOp(const QVariant & v, const QVariant & w) const
    { return v > w; }
    const char * _propertyName;
    bool (Compare::*_func)(const QVariant & v, const QVariant & w) const;
};

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

        QTreeView * view = new QTreeView(this);
        view->setModel(model);



    Optionnaly, user can declare property name used to order objects
    and a function to compute order value of the tree item. By default an internal function is used.

        model->setOrderingRole("zValue");

    Important to note that if ordering role is set, all objects should have this propery!
    If default ordering role is used, than objects are ordered by their index in the children list of the parent.
    In this case, the drag and drop of an object placing it between two objects will not work as expected.

 */

//******************************************************************************

const QEvent::Type ObjectTreeModel::ChildConstructed = (QEvent::Type) QEvent::registerEventType();

/*!
 * \brief ObjectTreeModel::ObjectTreeModel
 * \param root is the invisible root of the QObject parent/children tree structure
 * \param parent
 */
ObjectTreeModel::ObjectTreeModel(QObject * root, QObject *parent) :
    QAbstractItemModel(parent),
    _root(root),
    _sortOrder(Qt::DescendingOrder)
{

    // install event filter on the whole tree of children
    QObjectList stack = QObjectList();
    stack << _root;
    while(!stack.isEmpty())
    {
        QObject * object = stack.takeLast();
        object->installEventFilter(this);
        QObjectList children = object->children();
        _parentChildrenMap[object] = children;
        stack << children;
    }
}

//******************************************************************************

void ObjectTreeModel::setRole(int role, const QString &propertyName)
{
    _rolePropertyMap.insert(role, propertyName);
}

//******************************************************************************

void ObjectTreeModel::setOrderingRole(const QString &propertyName, Qt::SortOrder order)
{
    if ((_orderingRole != propertyName ||
         order != _sortOrder) &&
            !propertyName.isEmpty())
    {
        _orderingRole = propertyName.toLatin1();

        // setup parentChildrenMap
        const Compare & comp = Compare(orderPropName(),
                               _sortOrder == Qt::DescendingOrder ? Compare::Greater : Compare::Less);
        foreach( QObject * parent, _parentChildrenMap.keys())
        {
            QObjectList & children = _parentChildrenMap[parent];
            std::sort(children.begin(), children.end(), comp);
        }
    }
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

    if (parentObject &&
            _parentChildrenMap.contains(parentObject) &&
            row >= 0 &&
            row < _parentChildrenMap[parentObject].count())
    {
        SD_TRACE_PTR("index : object", _parentChildrenMap[parentObject].at(row));
        return createIndex(row, column, _parentChildrenMap[parentObject].at(row));
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

    if (!_parentChildrenMap.contains(parentObject2))
    {
        SD_TRACE("ObjectTreeModel::parent : no parent in the map");
        return QModelIndex();
    }
    int row = _parentChildrenMap[parentObject2].indexOf(parentObject);
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
            QVariant out = object->property(propertyName.toLatin1().data());
            if (!out.isValid())
            {
                IfDefaultDisplayRole();
//                SD_TRACE1("ObjectTreeModel::data : property %1 is invalid", propertyName);
                return QVariant();
            }

            // DEBUG :
            if (role == Qt::DisplayRole)
            {
                QString s = out.toString();
                s += QString(", z=%1").arg(object->property(orderPropName()).toDouble());
                out = QVariant(s);
            }

//            const QMetaObject * metaObject = object->metaObject();
//            int propertyIndex = metaObject->indexOfProperty(propertyName.toLatin1().data());
//            if (propertyIndex < 0)
//            {
//                IfDefaultDisplayRole();
////                SD_TRACE1("ObjectTreeModel::data : property %1 is not found", propertyName);
//                return QVariant();
//            }
//            QMetaProperty property = metaObject->property(propertyIndex);
//            if (!property.isValid())
//            {
//                IfDefaultDisplayRole();
////                SD_TRACE("ObjectTreeModel::data : meta property is invalid");
//                return QVariant();
//            }

//            QVariant out = property.read(object);
            // Special process for boolean type
            if (/*out.isValid() &&*/ out.type() == QVariant::Bool)
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
            // Check if property exists:
            if (!object->property(propertyName.toLatin1().data()).isValid())
            {
                IfDefaultEditRole();
//                SD_TRACE1("ObjectTreeModel::data : property %1 is not found", propertyName);
                return false;
            }

//            const QMetaObject * metaObject = object->metaObject();
//            int propertyIndex = metaObject->indexOfProperty(propertyName.toLatin1().data());
//            if (propertyIndex < 0)
//            {
//                IfDefaultEditRole();
////                SD_TRACE1("ObjectTreeModel::data : property %1 is not found", propertyName);
//                return false;
//            }
//            QMetaProperty property = metaObject->property(propertyIndex);
//            if (!property.isValid())
//            {
//                IfDefaultEditRole();
//////                SD_TRACE("ObjectTreeModel::data : meta property is invalid");
//                return false;
//            }

//            if (!property.isWritable())
//            {
//                SD_TRACE("ObjectTreeModel::setData : meta property is not writable");
//                return false;
//            }

            // Special process for boolean type
            QVariant out = value;
            if (role == Qt::CheckStateRole)
            {
                Qt::CheckState s = (Qt::CheckState) out.value<int>();
                out = QVariant(s == Qt::Checked ? true : false);
            }

//            if (!property.write(object, out))
            if (!object->setProperty(propertyName.toLatin1().data(), out))
            {
                SD_TRACE1("ObjectTreeModel::setData : failed to write property \'%1\'", propertyName);
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

        SD_TRACE_PTR("- Child : ", child);
        SD_TRACE_PTR("- Child added to object", object);

        // check if added child exists in the model :
//        if (!_parentChildrenMap.contains(child))
//        {
//            // If child has just been created, we cannot check if new child has
//            // ordering property, because child as QObject is not fully constructed

//            // post an event to the object to handle
//            // at handling time the child will be fully contructed
//            QEvent * e = new QEvent(ObjectTreeModel::ChildConstructed);
//            QCoreApplication::postEvent(child, e);
//        }
//        else
//        {
        if (!_parentChildrenMap.contains(object))
        {
            // initialize children list for the added child
            _parentChildrenMap[object] = object->children();
        }
        onObjectReparented(child, object);
//        }
    }
    else if (event->type() == ObjectTreeModel::ChildConstructed)
    {
        SD_TRACE_PTR("ObjectTreeModel::ChildConstructed : object=", object);
        // Now added child is fully contructed :
        if (!_parentChildrenMap.contains(object))
        {
            // initialize children list for the added child
            _parentChildrenMap[object] = object->children();
        }
        QObject * parent = object->parent();
        if (!_orderingRole.isEmpty() &&
                !object->property(orderPropName()).isValid())
        {
            SD_TRACE("ObjectTreeModel::eventFilter : Added child does not have specified ordering property. It is added as dynamic property with a default value of the parent");
            object->setProperty(orderPropName(),parent->property(orderPropName()));
        }
        onObjectReparented(object, parent);
    }
    else if (event->type() == QEvent::ChildRemoved)
    {
        QChildEvent * childEvent = static_cast<QChildEvent*>(event);
        QObject * child = childEvent->child();
        child->removeEventFilter(this);

        SD_TRACE_PTR("- Child : ", child);
        SD_TRACE_PTR("- Child removed from object", object);
        QObject * grandparent = object->parent();
        int row = 0;
        if (grandparent && _parentChildrenMap.contains(grandparent))
        {
            row = _parentChildrenMap[grandparent].indexOf(object);
        }
        QModelIndex parent = createIndex(row, 0, object);

        if (!_parentChildrenMap.contains(object))
        {
            SD_TRACE_PTR("ObjectTreeModel::eventFilter : Object is not in the parent-children map. object ptr=", object);
            return false;
        }
        _parentChildrenMap.remove(child);
        QObjectList & children = _parentChildrenMap[object];
        row = children.indexOf(child);
        children.removeAt(row);
//        // Inform that all children have been removed <= because we can not indicate from which row a child has been removed
//        int count = object->children().count();
//        if (count == 0) count = 1;
        beginRemoveRows(parent, row, row);
        endRemoveRows();
    }

    return QAbstractItemModel::eventFilter(object, event);
}

//******************************************************************************

void ObjectTreeModel::onObjectReparented(QObject *child, QObject *parent)
{
    // insert into parentChildrenMap
    if (!_parentChildrenMap.contains(parent))
    {
        SD_TRACE_PTR("ObjectTreeModel::onObjectReparented : Object is not in the parent-children map. object ptr=", parent);
        return;
    }
    // get children of the parent
    QObjectList & children = _parentChildrenMap[parent];

    if (children.contains(child))
    {
        SD_TRACE("ObjectTreeModel::onObjectReparented : Child is already inserted !!!");
        return;
    }
    // insert the child according
    children.append(child);
    // find parent's model index :
    int row = children.indexOf(child);
    QModelIndex parentIndex = this->parent(createIndex(row, 0, child));
//        SD_TRACE_PTR("-- Child added : parent == object", parent.internalPointer());
//        SD_TRACE2("-- Child added : parent.row=%1, parent.column=%2", parent.row(), parent.column());
//        SD_TRACE1("-- new object position=%1", row);
    beginInsertRows(parentIndex, row, row);
    endInsertRows();
}

//******************************************************************************

}
