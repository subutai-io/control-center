#ifndef SSTREEMODEL_H
#define SSTREEMODEL_H

#include <QList>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>
#include "RestWorker.h"
#include "TreeViewItem.h"

class ITreeModel : public QAbstractItemModel {

protected:
  CTreeModelItem* m_root;
  virtual void init_model() = 0;

public:
  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
  QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
  int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
  int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
};

#endif // SSTREEMODEL_H

