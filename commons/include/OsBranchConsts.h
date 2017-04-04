#ifndef OSBRANCHCONSTS_H
#define OSBRANCHCONSTS_H

#include <QString>

enum branch_t {
  BT_MASTER = 0,
  BT_STAGE = 1,
  BT_DEV = 2
};

enum os_t {
  OS_LINUX = 0,
  OS_WIN = 1,
  OS_MAC = 2
};

template<branch_t v> struct Branch2Type {
  enum {val = v};
};

template<os_t v> struct Os2Type {
  enum {val = v};
};
////////////////////////////////////////////////////////////////////////////

const QString& tray_kurjun_file_name();
const QString& p2p_kurjun_file_name();
const QString& p2p_dht_arg();

const QString& hub_post_url();
const QString& hub_get_url();
const QString& hub_health_url();
const QString& hub_gorjun_url();

const char* ssdp_rh_search_target();

const QString& default_p2p_path();
const QString& default_terminal();
const QString& default_term_arg();

#endif // OSBRANCHCONSTS_H
