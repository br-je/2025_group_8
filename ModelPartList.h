/**
 * @file ModelPartList.h
 * @brief Defines the ModelPartList class used as the Qt model for displaying
 *        model parts in a tree view.
 *
 * This class manages the hierarchy of ModelPart objects and provides the
 * required interface for Qt's QTreeView.
 */

#ifndef VIEWER_MODELPARTLIST_H
#define VIEWER_MODELPARTLIST_H

#include "ModelPart.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QList>

class ModelPart;

/**
 * @brief Model class used by Qt to display ModelPart objects in a tree view.
 *
 * Implements QAbstractItemModel to provide hierarchical data for the GUI.
 */
class ModelPartList : public QAbstractItemModel {
    Q_OBJECT

public:

    /**
     * @brief Constructor
     * @param data Initial data (not heavily used)
     * @param parent Parent QObject
     */
    ModelPartList(const QString& data, QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~ModelPartList();

    /**
     * @brief Returns number of columns in the model.
     * @param parent Parent index (unused)
     * @return Number of columns (e.g. Part, Visible)
     */
    int columnCount(const QModelIndex& parent) const;

    /**
     * @brief Returns data for a given index and role.
     * @param index Specifies row and column
     * @param role Specifies how data is used (display/edit)
     * @return Data value as QVariant
     */
    QVariant data(const QModelIndex& index, int role) const;

    /**
     * @brief Returns flags for an item.
     * @param index Item index
     * @return Qt item flags (e.g. selectable, editable)
     */
    Qt::ItemFlags flags(const QModelIndex& index) const;

    /**
     * @brief Returns header data for the model.
     * @param section Column index
     * @param orientation Horizontal/vertical
     * @param role Data role
     * @return Header text
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    /**
     * @brief Creates an index for a given row and column.
     * @param row Row index
     * @param column Column index
     * @param parent Parent index
     * @return QModelIndex for the item
     */
    QModelIndex index(int row, int column, const QModelIndex& parent) const;

    /**
     * @brief Returns parent index of a given item.
     * @param index Child index
     * @return Parent QModelIndex
     */
    QModelIndex parent(const QModelIndex& index) const;

    /**
     * @brief Returns number of child rows under a parent.
     * @param parent Parent index
     * @return Number of child items
     */
    int rowCount(const QModelIndex& parent) const;

    /**
     * @brief Returns pointer to root item.
     * @return Root ModelPart pointer
     */
    ModelPart* getRootItem();

    /**
     * @brief Adds a child item to the tree.
     * @param parent Parent index
     * @param data Data for new child
     * @return QModelIndex of new item
     */
    QModelIndex appendChild(QModelIndex& parent, const QList<QVariant>& data);

    /**
     * @brief Removes rows from the model.
     * @param row Starting row
     * @param count Number of rows to remove
     * @param parent Parent index
     * @return True if removal successful
     */
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:

    /** Pointer to the root item of the tree */
    ModelPart* rootItem;
};

#endif
