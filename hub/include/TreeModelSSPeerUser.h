#ifndef CTREEMODELSSPEERUSER_H
#define CTREEMODELSSPEERUSER_H

#include <QList>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>
#include "RestWorker.h"
#include "TreeViewItem.h"
#include "ITreeModel.h"

class CTreeModelSSPeerUser : public ITreeModel
{
private:
  CSSPeerUser m_user;

  protected:
  virtual void init_model();

public:
  CTreeModelSSPeerUser(const CSSPeerUser& user);
  virtual ~CTreeModelSSPeerUser();

  bool peer_user_changed(const CSSPeerUser& user) {
    return m_user != user;
  }
};

#endif // CTREEMODELSSPEERUSER_H
