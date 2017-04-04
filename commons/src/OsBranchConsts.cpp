#include "OsBranchConsts.h"

template<class BR, class OS> const QString& p2p_kurjun_file_name_temp_internal();

#define p2p_kurjun_file_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& p2p_kurjun_file_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

p2p_kurjun_file_name_def(BT_MASTER,     OS_LINUX,   "p2p")
p2p_kurjun_file_name_def(BT_MASTER,     OS_MAC,     "p2p_osx")
p2p_kurjun_file_name_def(BT_MASTER,     OS_WIN,     "p2p.exe")
p2p_kurjun_file_name_def(BT_DEV,        OS_LINUX,   "p2p")
p2p_kurjun_file_name_def(BT_DEV,        OS_MAC,     "p2p_osx")
p2p_kurjun_file_name_def(BT_DEV,        OS_WIN,     "p2p.exe")
p2p_kurjun_file_name_def(BT_STAGE,      OS_LINUX,   "p2p")
p2p_kurjun_file_name_def(BT_STAGE,      OS_MAC,     "p2p_osx")
p2p_kurjun_file_name_def(BT_STAGE,      OS_WIN,     "p2p.exe")

const QString &
p2p_kurjun_file_name() {
  return p2p_kurjun_file_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR, class OS> const QString& tray_kurjun_file_name_temp_internal();

#define tray_kurjun_file_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& tray_kurjun_file_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

tray_kurjun_file_name_def(BT_MASTER,    OS_LINUX,   "SubutaiTray")
tray_kurjun_file_name_def(BT_MASTER,    OS_MAC,     "SubutaiTray_osx")
tray_kurjun_file_name_def(BT_MASTER,    OS_WIN,     "SubutaiTray.exe")
tray_kurjun_file_name_def(BT_DEV,       OS_LINUX,   "SubutaiTray")
tray_kurjun_file_name_def(BT_DEV,       OS_MAC,     "SubutaiTray_osx")
tray_kurjun_file_name_def(BT_DEV,       OS_WIN,     "SubutaiTray.exe")
tray_kurjun_file_name_def(BT_STAGE,     OS_LINUX,   "SubutaiTray")
tray_kurjun_file_name_def(BT_STAGE,     OS_MAC,     "SubutaiTray_osx")
tray_kurjun_file_name_def(BT_STAGE,     OS_WIN,     "SubutaiTray.exe")

const QString &
tray_kurjun_file_name() {
  return tray_kurjun_file_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_post_url_temp_internal();

#define hub_post_url_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_post_url_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_post_url_temp_internal_def(BT_MASTER, "https://hub.subut.ai/rest/v1/tray/%1")
hub_post_url_temp_internal_def(BT_STAGE,  "https://stage.subut.ai/rest/v1/tray/%1")
hub_post_url_temp_internal_def(BT_DEV,    "https://dev.subut.ai/rest/v1/tray/%1")

const QString &
hub_post_url() {
  return hub_post_url_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_get_url_temp_internal();

#define hub_get_url_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_get_url_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_get_url_temp_internal_def(BT_MASTER, "https://hub.subut.ai/rest/v1/tray/%1")
hub_get_url_temp_internal_def(BT_STAGE,  "https://stage.subut.ai/rest/v1/tray/%1")
hub_get_url_temp_internal_def(BT_DEV,    "https://dev.subut.ai/rest/v1/tray/%1")

const QString &
hub_get_url() {
  return hub_get_url_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_healt_url_temp_internal();

#define hub_health_url_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_healt_url_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_health_url_temp_internal_def(BT_MASTER, "https://hub.subut.ai/rest/v1/tray/tray-data")
hub_health_url_temp_internal_def(BT_STAGE,  "https://stage.subut.ai/rest/v1/tray/tray-data")
hub_health_url_temp_internal_def(BT_DEV,    "https://dev.subut.ai/rest/v1/tray/tray-data")

const QString &
hub_health_url() {
  return hub_healt_url_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_kurjun_url_temp_internal();

#define hub_kurjun_url_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_kurjun_url_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_kurjun_url_temp_internal_def(BT_MASTER, "https://cdn.subut.ai:8338/kurjun/rest/%1")
hub_kurjun_url_temp_internal_def(BT_STAGE,  "https://stagecdn.subut.ai:8338/kurjun/rest/%1")
hub_kurjun_url_temp_internal_def(BT_DEV,    "https://devcdn.subut.ai:8338/kurjun/rest/%1")

const QString &
hub_gorjun_url() {
  return hub_kurjun_url_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const char* ssdp_rh_search_target_temp_internal();

#define ssdp_rh_search_target_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const char* ssdp_rh_search_target_temp_internal<Branch2Type<BT_TYPE> >() { \
    return STRING; \
  }

ssdp_rh_search_target_temp_internal_def(BT_MASTER, "urn:subutai-master:management:peer:4")
ssdp_rh_search_target_temp_internal_def(BT_STAGE,  "urn:subutai:management:peer:4")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    "urn:subutai-dev:management:peer:4")

const char *
ssdp_rh_search_target() {
  return ssdp_rh_search_target_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& p2p_dht_arg_temp_internal();

#define p2p_dht_arg_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& p2p_dht_arg_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

p2p_dht_arg_internal_def(BT_MASTER, "mastercdn.subut.ai:6881")
p2p_dht_arg_internal_def(BT_STAGE,  "dht.subut.ai:6881")
p2p_dht_arg_internal_def(BT_DEV,    "devcdn.subut.ai:6881")

const QString &
p2p_dht_arg() {
  return p2p_dht_arg_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& default_p2p_path_temp_internal();

#define default_p2p_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_p2p_path_temp_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

default_p2p_path_internal_def(OS_LINUX, "/opt/subutai/bin/p2p")
default_p2p_path_internal_def(OS_WIN, "p2p.exe")
default_p2p_path_internal_def(OS_MAC, "/Applications/Subutai/p2p")

const QString &
default_p2p_path() {
  return default_p2p_path_temp_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& default_terminal_temp_internal();

#define default_terminal_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_terminal_temp_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

default_terminal_internal_def(OS_LINUX, "xterm")
default_terminal_internal_def(OS_MAC, "osascript")
default_terminal_internal_def(OS_WIN, "cmd")

const QString &
default_terminal() {
  return default_terminal_temp_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& default_term_arg_temp_internal();

#define default_term_arg_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_term_arg_temp_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

default_term_arg_internal_def(OS_LINUX, "-e")
default_term_arg_internal_def(OS_MAC, "-e")
default_term_arg_internal_def(OS_WIN, "/k")

const QString &
default_term_arg() {
  return default_term_arg_temp_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////
