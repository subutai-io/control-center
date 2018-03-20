#include "OsBranchConsts.h"

#include <QApplication>
#include <QDir>

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
p2p_kurjun_file_name_def(BT_PROD,      OS_LINUX,   "p2p")
p2p_kurjun_file_name_def(BT_PROD,      OS_MAC,     "p2p_osx")
p2p_kurjun_file_name_def(BT_PROD,      OS_WIN,     "p2p.exe")

const QString &
p2p_kurjun_file_name() {
  return p2p_kurjun_file_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////
template<class BR, class OS> const QString& p2p_kurjun_package_name_temp_internal();

#define p2p_kurjun_package_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& p2p_kurjun_package_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

p2p_kurjun_package_name_def(BT_MASTER,     OS_MAC,     "subutai-p2p-master.pkg")
p2p_kurjun_package_name_def(BT_MASTER,     OS_WIN,     "subutai-p2p-master.msi")
p2p_kurjun_package_name_def(BT_MASTER,     OS_LINUX,   "subutai-p2p-master.deb")
p2p_kurjun_package_name_def(BT_DEV,        OS_LINUX,   "subutai-p2p-dev.deb")
p2p_kurjun_package_name_def(BT_DEV,        OS_MAC,     "subutai-p2p-dev.pkg")
p2p_kurjun_package_name_def(BT_DEV,        OS_WIN,     "subutai-p2p-dev.msi")
p2p_kurjun_package_name_def(BT_PROD,      OS_LINUX,    "subutai-p2p.deb")
p2p_kurjun_package_name_def(BT_PROD,      OS_MAC,      "subutai-p2p.pkg")
p2p_kurjun_package_name_def(BT_PROD,      OS_WIN,      "subutai-p2p.msi")

const QString &
p2p_kurjun_package_name() {
  return p2p_kurjun_package_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}
//////////////////////////////////////////////////////////////////////////////////////////
template<class BR, class OS> const QString& x2go_kurjun_package_name_temp_internal();

#define x2go_kurjun_package_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& x2go_kurjun_package_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

x2go_kurjun_package_name_def(BT_MASTER,     OS_MAC,     "X2GoClient_latest_macosx_10_9.dmg")
x2go_kurjun_package_name_def(BT_MASTER,     OS_WIN,     "X2GoClient_latest_mswin32-setup.exe")
x2go_kurjun_package_name_def(BT_MASTER,     OS_LINUX,   "subutai-p2p-master.deb")
x2go_kurjun_package_name_def(BT_DEV,        OS_LINUX,   "subutai-p2p-dev.deb")
x2go_kurjun_package_name_def(BT_DEV,        OS_MAC,     "X2GoClient_latest_macosx_10_9.dmg")
x2go_kurjun_package_name_def(BT_DEV,        OS_WIN,     "X2GoClient_latest_mswin32-setup.exe")
x2go_kurjun_package_name_def(BT_PROD,      OS_LINUX,    "subutai-p2p.deb")
x2go_kurjun_package_name_def(BT_PROD,      OS_MAC,      "X2GoClient_latest_macosx_10_9.dmg")
x2go_kurjun_package_name_def(BT_PROD,      OS_WIN,      "X2GoClient_latest_mswin32-setup.exe")

const QString &
x2go_kurjun_package_name(){
    return x2go_kurjun_package_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////
template<class BR, class OS> const QString& vagrant_kurjun_package_name_temp_internal();

#define vagrant_kurjun_package_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& vagrant_kurjun_package_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

vagrant_kurjun_package_name_def(BT_MASTER,     OS_MAC,     "vagrant.pkg")
vagrant_kurjun_package_name_def(BT_MASTER,     OS_WIN,     "vagrant_2.0.3_x86_64.msi")
vagrant_kurjun_package_name_def(BT_MASTER,     OS_LINUX,   "vagrant_2.0.3_x86_64.deb")
vagrant_kurjun_package_name_def(BT_DEV,        OS_LINUX,   "vagrant_2.0.3_x86_64.deb")
vagrant_kurjun_package_name_def(BT_DEV,        OS_MAC,     "vagrant.pkg")
vagrant_kurjun_package_name_def(BT_DEV,        OS_WIN,     "vagrant_2.0.3_x86_64.msi")
vagrant_kurjun_package_name_def(BT_PROD,      OS_LINUX,    "vagrant_2.0.3_x86_64.deb")
vagrant_kurjun_package_name_def(BT_PROD,      OS_MAC,      "vagrant.pkg")
vagrant_kurjun_package_name_def(BT_PROD,      OS_WIN,      "vagrant_2.0.3_x86_64.msi")

const QString &
vagrant_kurjun_package_name(){
    return vagrant_kurjun_package_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////
template<class BR, class OS> const QString& tray_kurjun_file_name_temp_internal();

#define tray_kurjun_file_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& tray_kurjun_file_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

tray_kurjun_file_name_def(BT_MASTER,    OS_LINUX,   "SubutaiControlCenter")
tray_kurjun_file_name_def(BT_MASTER,    OS_MAC,     "SubutaiControlCenter_osx")
tray_kurjun_file_name_def(BT_MASTER,    OS_WIN,     "SubutaiControlCenter.exe")
tray_kurjun_file_name_def(BT_DEV,       OS_LINUX,   "SubutaiControlCenter")
tray_kurjun_file_name_def(BT_DEV,       OS_MAC,     "SubutaiControlCenter_osx")
tray_kurjun_file_name_def(BT_DEV,       OS_WIN,     "SubutaiControlCenter.exe")
tray_kurjun_file_name_def(BT_PROD,     OS_LINUX,   "SubutaiControlCenter")
tray_kurjun_file_name_def(BT_PROD,     OS_MAC,     "SubutaiControlCenter_osx")
tray_kurjun_file_name_def(BT_PROD,     OS_WIN,     "SubutaiControlCenter.exe")

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

hub_post_url_temp_internal_def(BT_PROD,   "https://bazaar.subutai.io/rest/v1/tray/%1")
hub_post_url_temp_internal_def(BT_MASTER, "https://masterbazaar.subutai.io/rest/v1/tray/%1")
hub_post_url_temp_internal_def(BT_DEV,    "https://devbazaar.subutai.io/rest/v1/tray/%1")

const QString &
hub_post_url() {
  return hub_post_url_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_register_url_temp_internal();

#define hub_register_url_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_register_url_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_register_url_temp_internal_def(BT_PROD,   "https://bazaar.subutai.io/register")
hub_register_url_temp_internal_def(BT_MASTER, "https://masterbazaar.subutai.io/register")
hub_register_url_temp_internal_def(BT_DEV,    "https://devbazaar.subutai.io/register")

const QString &
hub_register_url() {
  return hub_register_url_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_user_profile_temp_internal();

#define hub_user_profile_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_user_profile_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_user_profile_temp_internal_def(BT_PROD,   "https://bazaar.subutai.io/users/")
hub_user_profile_temp_internal_def(BT_MASTER, "https://masterbazaar.subutai.io/users/")
hub_user_profile_temp_internal_def(BT_DEV,    "https://devbazaar.subutai.io/users/")

const QString &
hub_user_profile_url() {
  return hub_user_profile_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_get_url_temp_internal();

#define hub_get_url_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_get_url_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_get_url_temp_internal_def(BT_PROD,    "https://bazaar.subutai.io/rest/v1/tray/%1")
hub_get_url_temp_internal_def(BT_MASTER,  "https://masterbazaar.subutai.io/rest/v1/tray/%1")
hub_get_url_temp_internal_def(BT_DEV,     "https://devbazaar.subutai.io/rest/v1/tray/%1")

const QString &
hub_get_url() {
  return hub_get_url_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}

////////////////////////////////////////////////////////////////////////////

const QString &
p2p_rest_url() {
  static QString p2p_rest_url_ret("http://127.0.0.1:52523/rest/v1/%1");
  return p2p_rest_url_ret;
}

////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_healt_url_temp_internal();

#define hub_health_url_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_healt_url_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_health_url_temp_internal_def(BT_PROD,   "https://bazaar.subutai.io/rest/v1/tray/tray-data")
hub_health_url_temp_internal_def(BT_MASTER, "https://masterbazaar.subutai.io/rest/v1/tray/tray-data")
hub_health_url_temp_internal_def(BT_DEV,    "https://devbazaar.subutai.io/rest/v1/tray/tray-data")

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

hub_kurjun_url_temp_internal_def(BT_PROD,     "https://cdn.subutai.io:8338/kurjun/rest/%1")
hub_kurjun_url_temp_internal_def(BT_MASTER,   "https://mastercdn.subutai.io:8338/kurjun/rest/%1")
hub_kurjun_url_temp_internal_def(BT_DEV,      "https://devcdn.subutai.io:8338/kurjun/rest/%1")

const QString &
hub_gorjun_url() {
  return hub_kurjun_url_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_billing_temp_internal();

#define hub_billing_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_billing_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_billing_temp_internal_def(BT_PROD,   "https://bazaar.subutai.io/users/%1")
hub_billing_temp_internal_def(BT_MASTER, "https://masterbazaar.subutai.io/users/%1")
hub_billing_temp_internal_def(BT_DEV,    "https://devbazaar.subutai.io/users/%1")

const QString &
hub_billing_url() {
  return hub_billing_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR, class VER> const char* ssdp_rh_search_target_temp_internal();

#define ssdp_rh_search_target_temp_internal_def(BT_TYPE, VERSION, STRING) \
  template<> \
  const char* ssdp_rh_search_target_temp_internal<Branch2Type<BT_TYPE> , Int2Type<VERSION> >() { \
    return STRING; \
  }

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 1, "urn:subutai-master:management:peer:1")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   1,  "urn:subutai:management:peer:1")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    1,  "urn:subutai-dev:management:peer:1")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 2, "urn:subutai-master:management:peer:2")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   2,  "urn:subutai:management:peer:2")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    2,  "urn:subutai-dev:management:peer:2")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 3, "urn:subutai-master:management:peer:3")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   3,  "urn:subutai:management:peer:3")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    3,  "urn:subutai-dev:management:peer:3")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 4, "urn:subutai-master:management:peer:4")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   4,  "urn:subutai:management:peer:4")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    4,  "urn:subutai-dev:management:peer:4")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 5, "urn:subutai-master:management:peer:5")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   5,  "urn:subutai:management:peer:5")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    5,  "urn:subutai-dev:management:peer:5")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 6, "urn:subutai-master:management:peer:6")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   6,  "urn:subutai:management:peer:6")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    6,  "urn:subutai-dev:management:peer:6")

const char **
ssdp_rh_search_target_arr() {
  static const char* targets[] = {
    ssdp_rh_search_target_temp_internal<Branch2Type<CURRENT_BRANCH>, Int2Type<1> >(),
    ssdp_rh_search_target_temp_internal<Branch2Type<CURRENT_BRANCH>, Int2Type<2> >(),
    ssdp_rh_search_target_temp_internal<Branch2Type<CURRENT_BRANCH>, Int2Type<3> >(),
    ssdp_rh_search_target_temp_internal<Branch2Type<CURRENT_BRANCH>, Int2Type<4> >(),
    ssdp_rh_search_target_temp_internal<Branch2Type<CURRENT_BRANCH>, Int2Type<5> >(),
    ssdp_rh_search_target_temp_internal<Branch2Type<CURRENT_BRANCH>, Int2Type<6> >(),
    NULL
  };
  return targets;
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& p2p_dht_arg_temp_internal();

#define p2p_dht_arg_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& p2p_dht_arg_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

p2p_dht_arg_internal_def(BT_MASTER, "eu0.mastercdn.subutai.io:6881")
p2p_dht_arg_internal_def(BT_PROD,   "eu0.cdn.subutai.io:6881")
p2p_dht_arg_internal_def(BT_DEV,    "eu0.devcdn.subutai.io:6881")

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
default_p2p_path_internal_def(OS_WIN, "C:\\ProgramData\\subutai\\bin\\p2p.exe")
default_p2p_path_internal_def(OS_MAC, "/usr/local/share/subutai/bin/p2p")

const QString &
default_p2p_path() {
  return default_p2p_path_temp_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& default_vagrant_path_temp_internal();

#define default_vagrant_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_vagrant_path_temp_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

default_vagrant_path_internal_def(OS_LINUX, "/usr/bin/vagrant")
default_vagrant_path_internal_def(OS_WIN, "C:\\HashiCorp\\Vagrant\\bin\\vagrant.exe")
default_vagrant_path_internal_def(OS_MAC, "/usr/local/bin/vagrant")

const QString &
default_vagrant_path() {
  return default_vagrant_path_temp_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////
template<class OS> const QString& default_oracle_virtualbox_path_temp_internal();

#define default_oracle_virtualbox_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_oracle_virtualbox_path_temp_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

default_oracle_virtualbox_path_internal_def(OS_LINUX, "/usr/bin/virtualbox")
default_oracle_virtualbox_path_internal_def(OS_WIN, "C:\\Program Files\\Oracle\\VirtualBox\\VirtualBox.exe")
default_oracle_virtualbox_path_internal_def(OS_MAC, "/usr/local/bin/virtualbox")

const QString & default_oracle_virtualbox_path() {
    return default_oracle_virtualbox_path_temp_internal<Os2Type<CURRENT_OS > >();
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
default_terminal_internal_def(OS_MAC, "Terminal")
default_terminal_internal_def(OS_WIN, "cmd")

const QString &
default_terminal() {
  return default_terminal_temp_internal<Os2Type<CURRENT_OS> >();
}

////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& default_x2goclient_path_internal();

#define default_x2goclient_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_x2goclient_path_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

default_x2goclient_path_internal_def(OS_LINUX, "/usr/bin/x2goclient")
default_x2goclient_path_internal_def(OS_MAC, "x2goclient")
default_x2goclient_path_internal_def(OS_WIN, "C:\\Program Files (x86)\\x2goclient\\x2goclient.exe")

const QString &
default_x2goclient_path() {
  return default_x2goclient_path_internal<Os2Type<CURRENT_OS> >();
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
default_term_arg_internal_def(OS_MAC, "do script")
default_term_arg_internal_def(OS_WIN, "/k")

const QString &
default_term_arg() {
  return default_term_arg_temp_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& ssh_keygen_cmd_path_internal();

#define ssh_keygen_cmd_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& ssh_keygen_cmd_path_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

ssh_keygen_cmd_path_internal_def(OS_LINUX, "/usr/bin/ssh-keygen")
ssh_keygen_cmd_path_internal_def(OS_MAC, "/usr/bin/ssh-keygen")
ssh_keygen_cmd_path_internal_def(OS_WIN, "C:\\Program Files (x86)\\ssh\\ssh-keygen.exe")

const QString &
ssh_keygen_cmd_path() {  
  return ssh_keygen_cmd_path_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& ssh_cmd_path_internal();

#define ssh_cmd_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& ssh_cmd_path_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

ssh_cmd_path_internal_def(OS_LINUX, "/usr/bin/ssh")
ssh_cmd_path_internal_def(OS_MAC, "/usr/bin/ssh")
ssh_cmd_path_internal_def(OS_WIN, "C:\\Program Files (x86)\\ssh\\ssh.exe")

const QString &
ssh_cmd_path() {
  return ssh_cmd_path_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& scp_cmd_path_internal();

#define scp_cmd_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& scp_cmd_path_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

scp_cmd_path_internal_def(OS_LINUX, "/usr/bin/scp")
scp_cmd_path_internal_def(OS_MAC, "/usr/bin/scp")
scp_cmd_path_internal_def(OS_WIN, "C:\\Program Files (x86)\\ssh\\scp.exe")

const QString &
scp_cmd_path() {
  return scp_cmd_path_internal<Os2Type<CURRENT_OS> >();
}

////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& hub_site_temp_internal();

#define hub_site_temp_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& hub_site_temp_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

hub_site_temp_internal_def(BT_PROD,   "https://bazaar.subutai.io")
hub_site_temp_internal_def(BT_MASTER, "https://masterbazaar.subutai.io")
hub_site_temp_internal_def(BT_DEV,    "https://devbazaar.subutai.io")

const QString &
hub_site() {
  return hub_site_temp_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& which_cmd_internal();

#define which_cmd_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& which_cmd_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

which_cmd_internal_def(OS_LINUX, "which")
which_cmd_internal_def(OS_MAC, "which")
which_cmd_internal_def(OS_WIN, "where")

const QString &
which_cmd() {
  return which_cmd_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& default_chrome_path_internal();

#define default_chrome_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_chrome_path_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

default_chrome_path_internal_def(OS_LINUX, "/usr/bin/google-chrome-stable")
default_chrome_path_internal_def(OS_MAC, "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome")
default_chrome_path_internal_def(OS_WIN, "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe")

const QString &
default_chrome_path() {
  return default_chrome_path_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& subutai_command_internal();

#define subutai_command_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& subutai_command_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

subutai_command_internal_def(BT_PROD,   "/snap/bin/subutai")
subutai_command_internal_def(BT_MASTER, "/snap/bin/subutai-master")
subutai_command_internal_def(BT_DEV,    "/snap/bin/subutai-dev")

const QString &
subutai_command() {
  return subutai_command_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& snap_p2p_path_internal();

#define snap_p2p_path_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& snap_p2p_path_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

snap_p2p_path_internal_def(BT_PROD,   "/snap/subutai/current/bin/p2p")
snap_p2p_path_internal_def(BT_MASTER, "/snap/subutai-master/current/bin/p2p")
snap_p2p_path_internal_def(BT_DEV,    "/snap/subutai-dev/current/bin/p2p")

const QString &
snap_p2p_path() {
  return snap_p2p_path_internal<Branch2Type<CURRENT_BRANCH> >();
}
////////////////////////////////////////////////////////////////////////////

template<class BR, class OS> const QString& p2p_package_url_temp_internal();

#define p2p_package_url_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& p2p_package_url_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

p2p_package_url_def(BT_MASTER,     OS_LINUX,   "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p-master.deb")
p2p_package_url_def(BT_MASTER,     OS_MAC,     "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p-master.pkg")
p2p_package_url_def(BT_MASTER,     OS_WIN,     "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p-master.msi")
p2p_package_url_def(BT_DEV,        OS_LINUX,   "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p-dev.deb")
p2p_package_url_def(BT_DEV,        OS_MAC,     "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p.deb")
p2p_package_url_def(BT_DEV,        OS_WIN,     "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p-dev.msi")
p2p_package_url_def(BT_PROD,       OS_LINUX,   "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p.deb")
p2p_package_url_def(BT_PROD,       OS_MAC,     "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p.pkg")
p2p_package_url_def(BT_PROD,       OS_WIN,     "https://cdn.subutai.io:8338/kurjun/rest/raw/get?name=subutai-p2p.msi")

const QString &
p2p_package_url() {
  return p2p_package_url_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}

const QString &
x2goclient_url() {
  static QString url = "https://wiki.x2go.org/doku.php/doc:installation:x2goclient";
  return url;
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& current_branch_name_temp_internal();

#define current_branch_name_def(BT_TYPE, STRING) \
  template<> \
  const QString& current_branch_name_temp_internal<Branch2Type<BT_TYPE>>() { \
    static QString res(STRING); \
    return res; \
  }

current_branch_name_def(BT_MASTER, QObject::tr("stage"))
current_branch_name_def(BT_DEV, QObject::tr("development"))
current_branch_name_def(BT_PROD, QObject::tr("production"))

const QString&
current_branch_name() {
    return current_branch_name_temp_internal<Branch2Type<CURRENT_BRANCH>>();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& branch_name_str_temp_internal();

#define branch_name_str_def(BT_TYPE, STRING) \
  template<> \
  const QString& branch_name_str_temp_internal<Branch2Type<BT_TYPE>>() { \
    static QString res(STRING); \
    return res; \
  }

branch_name_str_def(BT_MASTER, QObject::tr("-master"))
branch_name_str_def(BT_DEV, QObject::tr("-dev"))
branch_name_str_def(BT_PROD, QObject::tr(""))

const QString&
branch_name_str() {
    return branch_name_str_temp_internal<Branch2Type<CURRENT_BRANCH>>();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& base_interface_name_internal();

#define base_interface_name_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& base_interface_name_internal<Os2Type<OS_TYPE>>() { \
    static QString res(STRING); \
    return res; \
  }

base_interface_name_internal_def(OS_WIN, "windowsinterface")
base_interface_name_internal_def(OS_MAC, "tap")
base_interface_name_internal_def(OS_LINUX, "vptp")

const QString&
base_interface_name() {
    return base_interface_name_internal< Os2Type<CURRENT_OS> >();
}
