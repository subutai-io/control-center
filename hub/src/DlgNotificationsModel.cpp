#include "DlgNotificationsModel.h"
#include "Commons.h"

#include <QDebug>

enum {
  NTM_COL_DATE = 0,
  NTM_COL_LEVEL,
  NTM_COL_MSG,
  NTM_COL_COUNT
};

static QColor colors_by_level[4] = {QColor::fromRgb(145,255,200),
                          QColor::fromRgb(255,200,145),
                          QColor::fromRgb(255,145,145),
                          QColor::fromRgb(210,0,15)};

////////////////////////////////////////////////////////////////////////////

static void
sort_notification_by_date(std::vector<CNotification>* ds, bool asc) {
  std::sort(ds->begin(), ds->end(),
            [asc](const CNotification& l, const CNotification& r) {
    return asc ? l.date_time() < r.date_time() : l.date_time() > r.date_time();
  });
}

static void
sort_notification_by_level(std::vector<CNotification>* ds, bool asc) {
  std::sort(ds->begin(), ds->end(),
            [asc](const CNotification& l, const CNotification& r) {
    return asc ? l.level() < r.level() : l.level() > r.level();
  });
}

static void
sort_notification_by_message(std::vector<CNotification>* ds, bool asc) {
  std::sort(ds->begin(), ds->end(),
            [asc](const CNotification& l, const CNotification& r) {
    return asc ? l.message() < r.message() : l.message() > r.message();
  });
}

void
DlgNotificationSortProxyModel::sort(int column,
                                    Qt::SortOrder order) {
  if (column < NTM_COL_DATE || column >= NTM_COL_COUNT) return;
  void (*pf_sorters[])(std::vector<CNotification>*, bool) = {
    sort_notification_by_date,
    sort_notification_by_level,
    sort_notification_by_message
  };
  pf_sorters[column](&CNotificationLogger::Instance()->notifications(), order == Qt::AscendingOrder);
  this->invalidate();
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

  if (!index.isValid() || index.row() >= (int)m_ds->size())
    return QVariant();

  if (role == Qt::BackgroundColorRole)
    return QBrush(colors_by_level[(*m_ds)[index.row()].level()]);

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
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

enum {
  NUC_COUNT,
  NUC_LEVEL,
  NUC_MESSAGE,
  NUC_COL_COUNT
};

static void
sort_notification_unions_by_count(std::vector<CNotificationUnion>* ds,
                                  bool asc) {
  std::sort(ds->begin(), ds->end(),
            [asc](const CNotificationUnion& l, const CNotificationUnion& r) {
    return asc ? l.count() < r.count() : l.count() > r.count();
  });
}

static void
sort_notification_unions_by_level(std::vector<CNotificationUnion>* ds,
                                  bool asc) {
  std::sort(ds->begin(), ds->end(),
            [asc](const CNotificationUnion& l, const CNotificationUnion& r) {
    return asc ? l.level() < r.level() : l.level() > r.level();
  });
}

static void
sort_notification_unions_by_message(std::vector<CNotificationUnion>* ds,
                                  bool asc) {
  std::sort(ds->begin(), ds->end(),
            [asc](const CNotificationUnion& l, const CNotificationUnion& r) {
    return asc ? l.message() < r.message() : l.message() > r.message();
  });
}

void
DlgNotificationUnionSortProxyModel::sort(int column,
                                         Qt::SortOrder order) {
  if (column < NUC_COUNT || column >= NUC_COL_COUNT) return;
  void (*pf_sorters[])(std::vector<CNotificationUnion>*, bool) = {
    sort_notification_unions_by_count,
    sort_notification_unions_by_level,
    sort_notification_unions_by_message
  };
  pf_sorters[column](&CNotificationLogger::Instance()->notification_unions(), order == Qt::AscendingOrder);
  this->invalidate();
}
////////////////////////////////////////////////////////////////////////////

DlgNotificationUnionsTableModel::DlgNotificationUnionsTableModel(QObject *parent) :
  QAbstractTableModel(parent) {
  m_ds = &CNotificationLogger::Instance()->notification_unions();
}

DlgNotificationUnionsTableModel::~DlgNotificationUnionsTableModel() {

}
////////////////////////////////////////////////////////////////////////////

int
DlgNotificationUnionsTableModel::rowCount(const QModelIndex &parent) const {
  UNUSED_ARG(parent);
  return (int)m_ds->size();
}
////////////////////////////////////////////////////////////////////////////

int
DlgNotificationUnionsTableModel::columnCount(const QModelIndex &parent) const {
  UNUSED_ARG(parent);
  return NUC_COL_COUNT;
}
////////////////////////////////////////////////////////////////////////////

QVariant
DlgNotificationUnionsTableModel::data(const QModelIndex &index,
                                      int role) const {
  if (!index.isValid() || index.row() >= (int)m_ds->size())
    return QVariant();

  if (role == Qt::BackgroundColorRole)
    return QBrush(colors_by_level[(*m_ds)[index.row()].level()]);

  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  QVariant vals[] = {(*m_ds)[index.row()].count(),
                     (*m_ds)[index.row()].level_str(),
                     (*m_ds)[index.row()].message()};

  return vals[index.column()];
}

QVariant
DlgNotificationUnionsTableModel::headerData(int section,
                                            Qt::Orientation orientation,
                                            int role) const {
  static const QString hdrs[] = {"Count", "Level", "Message"};

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
DlgNotificationUnionsTableModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return Qt::ItemIsEnabled;
  return Qt::ItemIsEnabled;
}
////////////////////////////////////////////////////////////////////////////

