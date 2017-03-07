#include "OsBranchConsts.h"

template<class BR, class OS> const QString& p2p_kurjun_file_name_temp_internal();

#define p2p_kurjun_file_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& p2p_kurjun_file_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

p2p_kurjun_file_name_def(BT_MASTER,   OS_LINUX,   "p2p")
p2p_kurjun_file_name_def(BT_MASTER,   OS_MAC,     "p2p_osx")
p2p_kurjun_file_name_def(BT_MASTER,   OS_WIN,     "p2p.exe")
p2p_kurjun_file_name_def(BT_DEV,      OS_LINUX,   "p2p_dev")
p2p_kurjun_file_name_def(BT_DEV,      OS_MAC,     "p2p_osx_dev")
p2p_kurjun_file_name_def(BT_DEV,      OS_WIN,     "p2p_dev.exe")

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

tray_kurjun_file_name_def(BT_MASTER,  OS_LINUX,   "SubutaiTray")
tray_kurjun_file_name_def(BT_MASTER,  OS_MAC,     "SubutaiTray_osx")
tray_kurjun_file_name_def(BT_MASTER,  OS_WIN,     "SubutaiTray.exe")
tray_kurjun_file_name_def(BT_DEV,     OS_LINUX,   "SubutaiTray_dev")
tray_kurjun_file_name_def(BT_DEV,     OS_MAC,     "SubutaiTray_osx_dev")
tray_kurjun_file_name_def(BT_DEV,     OS_WIN,     "SubutaiTray_dev.exe")

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
