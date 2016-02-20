#include "TreeModelCSSContainer.h"

CTreeModelCSSContainer::CTreeModelCSSContainer(const CSSContainer &cont) :
  m_cont(cont)
{
  QList<QVariant> root_data;
  root_data << "Environment" << "Container name" << "Container description";
  m_root = new CTreeModelItem(root_data, 0);
  init_model();
}

CTreeModelCSSContainer::~CTreeModelCSSContainer()
{
}

void CTreeModelCSSContainer::init_model() {
  for (auto pair_it = m_cont.dct_env_containers().begin();
       pair_it != m_cont.dct_env_containers().end(); ++pair_it) {
    QList<QVariant> cont_item_values;
    cont_item_values << pair_it->first << "" << "";
    CTreeModelItem* cont_item = new CTreeModelItem(cont_item_values, m_root);

    for (auto cont_it = pair_it->second.begin();
         cont_it != pair_it->second.end(); ++cont_it) {
      QList<QVariant> cont_info;
      cont_info << "" << *cont_it << "";
      CTreeModelItem* cont_info_item = new CTreeModelItem(cont_info, cont_item);
      cont_item->appendChild(cont_info_item);
    }

    m_root->appendChild(cont_item);
  }
}
////////////////////////////////////////////////////////////////////////////




