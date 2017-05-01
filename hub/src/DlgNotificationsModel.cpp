#include "DlgNotificationsModel.h"
#include "Commons.h"

#include <QDebug>

enum {
  NTM_COL_DATE = 0,
  NTM_COL_LEVEL,
  NTM_COL_MSG,
  NTM_COL_COUNT
};
////////////////////////////////////////////////////////////////////////////

void
DlgNotificationSortProxyModel::sort(int column,
                                 Qt::SortOrder order) {
  if (column < NTM_COL_DATE || column >= NTM_COL_COUNT) return;

}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

DlgNotificationsTableModel::DlgNotificationsTableModel(QObject *parent) : QAbstractTableModel(parent) {
  m_ds = &CNotificationLogger::Instance()->notifications();
}

DlgNotificationsTableModel::~DlgNotificationsTableModel() {
}
////////////////////////////////////////////////////////////////////////////

int
DlgNotificationsTableModel::rowCount(const QModelIndex &parent) const {
  UNUSED_ARG(parent);
  return (int)m_ds->size();
}
////////////////////////////////////////////////////////////////////////////

int
DlgNotificationsTableModel::columnCount(const QModelIndex &parent) const {
  UNUSED_ARG(parent);
  return NTM_COL_COUNT;
}
////////////////////////////////////////////////////////////////////////////

QVariant
DlgNotificationsTableModel::data(const QModelIndex &index, int role) const {
  static QColor color[4] = {QColor::fromRgb(145,255,200),
                            QColor::fromRgb(255,200,145),
                            QColor::fromRgb(255,145,145),
                            QColor::fromRgb(210,0,15)};

  if (!index.isValid() || index.row() >= (int)m_ds->size())
    return QVariant();

  if (role == Qt::BackgroundColorRole)
    return QBrush(color[(*m_ds)[index.row()].level()]);

  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  QVariant vals[] = {(*m_ds)[index.row()].date_time(),
                     (*m_ds)[index.row()].level_str(),
                     (*m_ds)[index.row()].message()};

  return vals[index.column()];
}
////////////////////////////////////////////////////////////////////////////

QVariant
DlgNotificationsTableModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const {
  static const QString hdrs[] = {"Date", "Level", "Message"};

  if(role != Qt::DisplayRole)
    return QVariant();

  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    return hdrs[section];
  } else {
    return QString("%1").arg(section + 1); // возвращаем номера строк
  }
}
////////////////////////////////////////////////////////////////////////////

Qt::ItemFlags
DlgNotificationsTableModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return Qt::ItemIsEnabled;
  return Qt::ItemIsEnabled;
}
////////////////////////////////////////////////////////////////////////////
