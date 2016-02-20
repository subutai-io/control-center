#ifndef CTREEVIEWITEM_H
#define CTREEVIEWITEM_H

#include <QList>
#include <QVariant>

class CTreeModelItem {
private:
  QList<CTreeModelItem*> m_childItems;
  QList<QVariant> m_itemData;
  CTreeModelItem *m_parentItem;

public:
  CTreeModelItem(const QList<QVariant> &data,
                 CTreeModelItem *parent)   {
    m_parentItem = parent;
    m_itemData = data;
  }

  ~CTreeModelItem()   {
    qDeleteAll(m_childItems);
  }

  void appendChild(CTreeModelItem *item)   {
    m_childItems.append(item);
  }

  CTreeModelItem *child(int row) {
    return m_childItems.value(row);
  }

  int childCount() const {
    return m_childItems.count();
  }

  int columnCount() const {
    return m_itemData.count();
  }

  QVariant data(int column) const {
    return m_itemData.value(column);
  }

  CTreeModelItem *parentItem() {
    return m_parentItem;
  }

  int row() const {
    if (m_parentItem)
      return m_parentItem->m_childItems.indexOf(const_cast<CTreeModelItem*>(this));
    return 0;
  }
};

#endif // CTREEVIEWITEM_H

