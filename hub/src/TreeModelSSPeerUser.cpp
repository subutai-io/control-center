#include "TreeModelSSPeerUser.h"

CTreeModelSSPeerUser::CTreeModelSSPeerUser(const CSSPeerUser &user) :
  m_user(user) {
  QList<QVariant> root_data;
  root_data << "Peer" << "User name" << "User description";
  m_root = new CTreeModelItem(root_data, 0);
  init_model();
}

CTreeModelSSPeerUser::~CTreeModelSSPeerUser() {
  if (m_root) delete m_root;
}
////////////////////////////////////////////////////////////////////////////

void CTreeModelSSPeerUser::init_model() {

  for (auto pair_it = m_user.dct_peer_users().begin();
       pair_it != m_user.dct_peer_users().end(); ++pair_it) {
    QList<QVariant> peer_item_values;
    peer_item_values << pair_it->first << "" << "";
    CTreeModelItem* user_item = new CTreeModelItem(peer_item_values, m_root);

    for (auto user_iter = pair_it->second.begin();
         user_iter != pair_it->second.end(); ++user_iter) {
      QList<QVariant> cont_info;
      cont_info << "" << *user_iter << "";
      CTreeModelItem* cont_info_item = new CTreeModelItem(cont_info, user_item);
      user_item->appendChild(cont_info_item);
    }

    m_root->appendChild(user_item);
  }
}
////////////////////////////////////////////////////////////////////////////


