#ifndef CREEMODELCSSCONTAINER_H
#define CREEMODELCSSCONTAINER_H

#include <QList>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>
#include "RestWorker.h"
#include "TreeViewItem.h"
#include "ITreeModel.h"

class CTreeModelCSSContainer : public ITreeModel
{
private:
  CSSContainer m_cont;

protected:
  virtual void init_model();

public:
  CTreeModelCSSContainer(const CSSContainer& cont);
  virtual ~CTreeModelCSSContainer();

  bool container_changed(const CSSContainer& cont) {
    return m_cont != cont;
  }
};

#endif // CREEMODELCSSCONTAINER_H
