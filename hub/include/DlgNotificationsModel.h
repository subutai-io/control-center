#ifndef DLGNOTIFICATIONSMODEL_H
#define DLGNOTIFICATIONSMODEL_H

#include <vector>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QSortFilterProxyModel>
#include "NotificationLogger.h"

class DlgNotificationSortProxyModel : public QSortFilterProxyModel {
public:
  DlgNotificationSortProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent){}
  virtual ~DlgNotificationSortProxyModel() {}

protected:
  // QSortFilterProxyModel interface
  virtual void sort(int column, Qt::SortOrder order);
private:
};
////////////////////////////////////////////////////////////////////////////

class DlgNotificationsTableModel : public QAbstractTableModel {
  Q_OBJECT
private:
  const std::vector<CNotification>* m_ds;

public:
  explicit DlgNotificationsTableModel(QObject *parent = nullptr);
  virtual ~DlgNotificationsTableModel();

  virtual int rowCount(const QModelIndex &parent) const;
  virtual int columnCount(const QModelIndex &parent) const;
  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
};
////////////////////////////////////////////////////////////////////////////

#endif // DLGNOTIFICATIONSMODEL_H
