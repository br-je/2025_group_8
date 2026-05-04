/**
 * @file ModelPartList.h
 * @brief Defines the ModelPartList class used as the Qt model for the tree view.
 *
 * This class manages the hierarchy of ModelPart objects and provides data
 * to the QTreeView in the main application.
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
 * @brief Qt model used to display ModelPart objects in a tree structure.
 *
 * Provides the functions required by QAbstractItemModel so that CAD model
 * parts can be shown, selected, added, and removed in the tree view.
 */
class ModelPartList : public QAbstractItemModel {
    Q_OBJECT

public:
    /**
     * @brief Creates the model part list.
     * @param data Initial data for the model.
     * @param parent Parent QObject.
     */
    ModelPartList(const QString& data, QObject* parent = NULL);

    /**
     * @brief Deletes the model and frees the root item.
     */
    ~ModelPartList();

    /**
     * @brief Returns the number of columns in the tree model.
     * @param parent Parent index, unused in this model.
     * @return Number of columns in the tree view.
     */
    int columnCount(const QModelIndex& parent) const;

    /**
     * @brief Returns data for a given tree item.
     * @param index QModelIndex containing the requested row and column.
     * @param role Qt role specifying how the data should be used.
     * @return Data value for the requested index.
     */
    QVariant data(const QModelIndex& index, int role) const;

    /**
     * @brief Returns item flags for a tree item.
     * @param index QModelIndex of the requested item.
     * @return Qt item flags such as selectable or editable.
     */
    Qt::ItemFlags flags(const QModelIndex& index) const;

    /**
     * @brief Returns header data for the tree view.
     * @param section Header section index.
     * @param orientation Header orientation.
     * @param role Qt role specifying the requested data.
     * @return Header text or data.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    /**
     * @brief Creates a QModelIndex for a child item.
     * @param row Row number of the child.
     * @param column Column number.
     * @param parent Parent index.
     * @return QModelIndex for the requested item.
     */
    QModelIndex index(int row, int column, const QModelIndex& parent) const;

    /**
     * @brief Returns the parent index of a tree item.
     * @param index QModelIndex of the child item.
     * @return QModelIndex of the parent item.
     */
    QModelIndex parent(const QModelIndex& index) const;

    /**
     * @brief Returns the number of child rows under a parent.
     * @param parent Parent index.
     * @return Number of child items.
     */
    int rowCount(const QModelIndex& parent) const;

    /**
     * @brief Returns the root item of the model tree.
     * @return Pointer to the root ModelPart.
     */
    ModelPart* getRootItem();

    /**
     * @brief Adds a child item under a parent item.
     * @param parent Parent index where the child should be added.
     * @param data Data for the new child item.
     * @return QModelIndex of the newly added child.
     */
    QModelIndex appendChild(QModelIndex& parent, const QList<QVariant>& data);

    /**
     * @brief Removes one or more rows from the tree model.
     * @param row Starting row to remove.
     * @param count Number of rows to remove.
     * @param parent Parent index containing the rows.
     * @return True if the rows were removed successfully.
     */
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    /** Root item at the base of the model tree. */
    ModelPart *rootItem;
};

#endif

