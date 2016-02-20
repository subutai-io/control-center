#include "ITreeModel.h"

QVariant ITreeModel::data(const QModelIndex &index,
                          int role) const {
  if (!index.isValid())
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();
  CTreeModelItem *item = static_cast<CTreeModelItem*>(index.internalPointer());
  return item->data(index.column());
}
////////////////////////////////////////////////////////////////////////////

Qt::ItemFlags ITreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return 0;
  return QAbstractItemModel::flags(index);
}
////////////////////////////////////////////////////////////////////////////

QVariant ITreeModel::headerData(int section,
                                Qt::Orientation orientation,
                                int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return m_root->data(section);
  return QVariant();
}
////////////////////////////////////////////////////////////////////////////

QModelIndex ITreeModel::index(int row,
                              int column,
                              const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent))
    return QModelIndex();
  CTreeModelItem *parent_item;
  parent_item = parent.isValid() ? static_cast<CTreeModelItem*>(parent.internalPointer()) : m_root;
  CTreeModelItem *childItem = parent_item->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}
////////////////////////////////////////////////////////////////////////////

QModelIndex ITreeModel::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return QModelIndex();
  CTreeModelItem *child_item = static_cast<CTreeModelItem*>(index.internalPointer());
  CTreeModelItem *parent_item = child_item->parentItem();

  if (parent_item == m_root)
    return QModelIndex();
  return createIndex(parent_item->row(), 0, parent_item);
}
////////////////////////////////////////////////////////////////////////////

int ITreeModel::rowCount(const QModelIndex &parent) const {
  CTreeModelItem *parent_item;
  if (parent.column() > 0)
    return 0;
  parent_item = parent.isValid() ? static_cast<CTreeModelItem*>(parent.internalPointer()) : m_root;
  return parent_item->childCount();
}
////////////////////////////////////////////////////////////////////////////

int ITreeModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return static_cast<CTreeModelItem*>(parent.internalPointer())->columnCount();
  else
    return m_root->columnCount();
}
////////////////////////////////////////////////////////////////////////////
