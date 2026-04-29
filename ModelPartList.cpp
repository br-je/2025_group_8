/**
 * @file ModelPartList.cpp
 * @brief Implementation of the ModelPartList class.
 *
 * Provides the data model for the Qt tree view, managing the
 * hierarchical structure of ModelPart objects.
 */

#include "ModelPartList.h"
#include "ModelPart.h"

ModelPartList::ModelPartList( const QString& data, QObject* parent ) : QAbstractItemModel(parent) {
    /* Have option to specify number of visible properties for each item in tree - the root item
     * acts as the column headers
     */
    rootItem = new ModelPart( { tr("Part"), tr("Visible?") } );
}



ModelPartList::~ModelPartList() {
    delete rootItem;
}


int ModelPartList::columnCount( const QModelIndex& parent ) const {
    Q_UNUSED(parent);

    return rootItem->columnCount();
}


QVariant ModelPartList::data( const QModelIndex& index, int role ) const {
    /* If the item index isnt valid, return a new, empty QVariant (QVariant is generic datatype
     * that could be any valid QT class) */
    if( !index.isValid() )
        return QVariant();

    /* Role represents what this data will be used for, we only need deal with the case
     * when QT is asking for data to create and display the treeview. Return a new,
     * empty QVariant if any other request comes through. */
    if (role != Qt::DisplayRole)
        return QVariant();

    /* Get a a pointer to the item referred to by the QModelIndex */
    ModelPart* item = static_cast<ModelPart*>( index.internalPointer() );

    /* Each item in the tree has a number of columns ("Part" and "Visible" in this
     * initial example) return the column requested by the QModelIndex */
    return item->data( index.column() );
}


Qt::ItemFlags ModelPartList::flags( const QModelIndex& index ) const {
    if( !index.isValid() )
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags( index );
}


QVariant ModelPartList::headerData( int section, Qt::Orientation orientation, int role ) const {
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return rootItem->data( section );

    return QVariant();
}


QModelIndex ModelPartList::index(int row, int column, const QModelIndex& parent) const {
    ModelPart* parentItem;

    if( !parent.isValid() || !hasIndex(row, column, parent) )
        parentItem = rootItem;              // default to selecting root
    else
        parentItem = static_cast<ModelPart*>(parent.internalPointer());

    ModelPart* childItem = parentItem->child(row);
    if( childItem )
        return createIndex(row, column, childItem);


    return QModelIndex();
}


QModelIndex ModelPartList::parent( const QModelIndex& index ) const {
    if (!index.isValid())
        return QModelIndex();

    ModelPart* childItem = static_cast<ModelPart*>(index.internalPointer());
    ModelPart* parentItem = childItem->parentItem();

    if( parentItem == rootItem )
        return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}


int ModelPartList::rowCount( const QModelIndex& parent ) const {
    ModelPart* parentItem;
    if( parent.column() > 0 )
        return 0;

    if( !parent.isValid() )
        parentItem = rootItem;
    else
        parentItem = static_cast<ModelPart*>(parent.internalPointer());

    return parentItem->childCount();
}


ModelPart* ModelPartList::getRootItem() {
    return rootItem;
}



QModelIndex ModelPartList::appendChild(QModelIndex& parent, const QList<QVariant>& data)
{
    ModelPart* parentPart = nullptr;

    if (parent.isValid())
        parentPart = static_cast<ModelPart*>(parent.internalPointer());
    else
        parentPart = rootItem;

    if (!parentPart)
        return QModelIndex();

    int newRow = parentPart->childCount();

    beginInsertRows(parent, newRow, newRow);

    ModelPart* childPart = new ModelPart(data, parentPart);
    parentPart->appendChild(childPart);

    endInsertRows();

    return createIndex(newRow, 0, childPart);
}


///** Remove a child item from the tree
 /* @param row is the index of the item to remove
  * @param count is the number of items to remove (starting at row)
  * @param parent is the parent of the item to remove, default is root
  * @return true if successful, false if not
	  */
bool ModelPartList::removeRows(int row, int count, const QModelIndex& parent)
{
    ModelPart* parentItem;

    if (parent.isValid())
        parentItem = static_cast<ModelPart*>(parent.internalPointer());
    else
        parentItem = rootItem;

    if (!parentItem)
        return false;

    beginRemoveRows(parent, row, row + count - 1);

    bool success = true;

    for (int i = 0; i < count; ++i)
    {
        if (!parentItem->removeChild(row))
        {
            success = false;
            break;
        }
    }

    endRemoveRows();

    return success;
}
