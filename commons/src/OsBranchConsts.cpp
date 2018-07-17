#include "OsBranchConsts.h"

#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QSysInfo>
#include "SystemCallWrapper.h"
#include "SettingsManager.h"

static std::map<QString, QString> virtual_package_codename = {
    {"xenial",   "virtualbox-xenial.deb"},
    {"bionic",   "virtualbox-bionic.deb"},
    {"zesty",    "virtualbox-zesty.deb"},
    {"yakkety",  "virtualbox-yakkety.deb"},
    {"trusty",   "virtualbox-trusty.deb"},
    {"stretch",  "virtualbox-stretch.deb"},
    {"jessie",   "virtualbox-jessie.deb"},
    {"wheezy",   "virtualbox-wheezy.deb"},
    {"artful",   "virtualbox-zesty.deb"},
    {"qiana",    "virtualbox-trusty.deb"}, //compatible with 14.04
    {"rebecca",  "virtualbox-trusty.deb"},
    {"rafaela",  "virtualbox-trusty.deb"},
    {"rosa",     "virtualbox-trusty.deb"},
    {"sarah",    "virtualbox-xenial.deb"}, //compatible with 16.04
    {"serena",   "virtualbox-xenial.deb"},
    {"sonya",    "virtualbox-xenial.deb"},
    {"sylvia",   "virtualbox-xenial.deb"},
    {"rara",     "virtualbox-bionic.deb"} //compatible with 18.04
};

template<class BR, class OS> const QString& p2p_kurjun_file_name_temp_internal();
template<class BR, class OS> const QString& p2p_package_name_temp_internal();

#define p2p_kurjun_file_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& p2p_kurjun_file_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

#define p2p_package_name_def(BT_TYPE, OS_TYPE, STRING) \
  template<> \
  const QString& p2p_package_name_temp_internal<Branch2Type<BT_TYPE>, Os2Type<OS_TYPE> >() { \
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

p2p_package_name_def(BT_MASTER, OS_LINUX, "subutai-p2p-master")
p2p_package_name_def(BT_DEV,    OS_LINUX, "subutai-p2p-dev")
p2p_package_name_def(BT_PROD,   OS_LINUX, "subutai-p2p")
p2p_package_name_def(BT_MASTER, OS_WIN,   "Subutai P2P")
p2p_package_name_def(BT_DEV,    OS_WIN,   "Subutai P2P")
p2p_package_name_def(BT_PROD,   OS_WIN,   "Subutai P2P")
p2p_package_name_def(BT_MASTER, OS_MAC,   "")
p2p_package_name_def(BT_DEV,    OS_MAC,   "")
p2p_package_name_def(BT_PROD,   OS_MAC,   "")

const QString &
p2p_kurjun_file_name() {
  return p2p_kurjun_file_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}

const QString &
p2p_package_name() {
  return p2p_package_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
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
const QString &
oracle_virtualbox_kurjun_package_name() {
    static std::vector <std::pair<QString , QString> > info;
    static QString kurjun_file = "not_found";
    if(info.empty())
        current_os_info(info);
    if(info.empty())
        return kurjun_file;
    //first check type of os
    QString type = info.begin()->second;
    if(type == "Windows")
        kurjun_file = "VirtualBox.exe";
    else if(type == "Mac")
        kurjun_file = "VirtualBox.pkg";
    if(kurjun_file != "not_found")
        return kurjun_file;
    if(info.size() < 2 || info[0].second != "Linux")
        return kurjun_file;
    QString codename = info[1].second;
    if(virtual_package_codename.find(codename) != virtual_package_codename.end())
        kurjun_file = virtual_package_codename[codename];
    return kurjun_file;
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
x2go_kurjun_package_name_def(BT_MASTER,     OS_LINUX,   "X2GoClient_latest_linux.deb")
x2go_kurjun_package_name_def(BT_DEV,        OS_LINUX,   "X2GoClient_latest_linux.deb")
x2go_kurjun_package_name_def(BT_DEV,        OS_MAC,     "X2GoClient_latest_macosx_10_9.dmg")
x2go_kurjun_package_name_def(BT_DEV,        OS_WIN,     "X2GoClient_latest_mswin32-setup.exe")
x2go_kurjun_package_name_def(BT_PROD,      OS_LINUX,    "X2GoClient_latest_linux.deb")
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
vagrant_kurjun_package_name_def(BT_MASTER,     OS_WIN,     "vagrant.msi")
vagrant_kurjun_package_name_def(BT_MASTER,     OS_LINUX,   "vagrant.deb")
vagrant_kurjun_package_name_def(BT_DEV,        OS_LINUX,   "vagrant.deb")
vagrant_kurjun_package_name_def(BT_DEV,        OS_MAC,     "vagrant.pkg")
vagrant_kurjun_package_name_def(BT_DEV,        OS_WIN,     "vagrant.msi")
vagrant_kurjun_package_name_def(BT_PROD,      OS_LINUX,    "vagrant.deb")
vagrant_kurjun_package_name_def(BT_PROD,      OS_MAC,      "vagrant.pkg")
vagrant_kurjun_package_name_def(BT_PROD,      OS_WIN,      "vagrant.msi")

const QString &
vagrant_kurjun_package_name(){
    return vagrant_kurjun_package_name_temp_internal<Branch2Type<CURRENT_BRANCH>, Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////
template <class OS> const QString& chrome_kurjun_package_name_internal();
#define chrome_kurjun_package_name_def(OS_TYPE, STRING) \
  template<> \
  const QString& chrome_kurjun_package_name_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }
chrome_kurjun_package_name_def(OS_MAC, "googlechrome.dmg")
chrome_kurjun_package_name_def(OS_LINUX, "google-chrome-stable_current_amd64.deb")
chrome_kurjun_package_name_def(OS_WIN, "ChromeSetup.exe")
const QString& chrome_kurjun_package_name(){
    return chrome_kurjun_package_name_internal <Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////
template <class OS> const QString& firefox_kurjun_package_name_internal();
#define firefox_kurjun_package_name_def(OS_TYPE, STRING) \
  template<> \
  const QString& firefox_kurjun_package_name_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }
firefox_kurjun_package_name_def(OS_MAC, "mozillafirefox.dmg")
firefox_kurjun_package_name_def(OS_LINUX, "mozilla-firefox-stable_current_amd64.deb")
firefox_kurjun_package_name_def(OS_WIN, "FirefoxSetup.exe")
const QString& firefox_kurjun_package_name(){
    return firefox_kurjun_package_name_internal <Os2Type<CURRENT_OS> >();
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
template<class BR> const QString& subutai_box_kurjun_package_name_internal(const QString& provider);
template<>
const QString& subutai_box_kurjun_package_name_internal<Branch2Type<BT_PROD> >(const QString& provider){
    static std::map<QString, QString> box_names = {{"vmware_desktop", "vagrant-subutai-stretch-vmware.box"},
                                                   {"parallels", "vagrant-subutai-stretch-parallels.box"},
                                                   {"hyperv", "vagrant-subutai-stretch-hyperv.box"},
                                                   {"libvirt", "vagrant-subutai-stretch-libvirt.box"},
                                                   {"virtualbox", "vagrant-subutai-stretch-virtualbox.box"},
                                                   {"undefined", "undefined"}};
    if(box_names.find(provider) == box_names.end()){
        return box_names["undefined"];
    }
    return box_names[provider];
}
template<>
const QString& subutai_box_kurjun_package_name_internal<Branch2Type<BT_MASTER> >(const QString& provider){
    static std::map<QString, QString> box_names = {{"vmware_desktop", "vagrant-subutai-stretch-vmware-master.box"},
                                                   {"parallels", "vagrant-subutai-stretch-parallels-master.box"},
                                                   {"hyperv", "vagrant-subutai-stretch-hyperv-master.box"},
                                                   {"libvirt", "vagrant-subutai-stretch-libvirt-master.box"},
                                                   {"virtualbox", "vagrant-subutai-stretch-virtualbox-master.box"},
                                                   {"undefined", "undefined"}};
    if(box_names.find(provider) == box_names.end()){
        return box_names["undefined"];
    }
    return box_names[provider];
}
template<>
const QString& subutai_box_kurjun_package_name_internal<Branch2Type<BT_DEV> >(const QString& provider){
    static std::map<QString, QString> box_names = {{"vmware_desktop", "vagrant-subutai-stretch-vmware.box"},
                                                   {"parallels", "vagrant-subutai-stretch-parallels.box"},
                                                   {"hyperv", "vagrant-subutai-stretch-hyperv.box"},
                                                   {"libvirt", "vagrant-subutai-stretch-libvirt.box"},
                                                   {"virtualbox", "vagrant-subutai-stretch-virtualbox.box"},
                                                   {"undefined", "undefined"}};
    if(box_names.find(provider) == box_names.end()){
        return box_names["undefined"];
    }
    return box_names[provider];
}
const QString& subutai_box_kurjun_package_name(const QString& provider){
    return subutai_box_kurjun_package_name_internal<Branch2Type<CURRENT_BRANCH> >(provider);
}
////////////////////////////////////////////////////////////////////////////
template<class BR> const QString& subutai_box_name_internal();

#define subutai_box_name_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& subutai_box_name_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

subutai_box_name_internal_def(BT_PROD,   "subutai/stretch")
subutai_box_name_internal_def(BT_MASTER, "subutai/stretch-master")
subutai_box_name_internal_def(BT_DEV,    "subutai/stretch")

const QString &
subutai_box_name() {
  const QString branch = set_application_branch();
  if (branch == "production")
    return subutai_box_name_internal<Branch2Type<BT_PROD> >();
  else if (branch == "stage")
    return subutai_box_name_internal<Branch2Type<BT_MASTER> >();
  else
    return subutai_box_name_internal<Branch2Type<BT_DEV> >();
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return hub_post_url_temp_internal<Branch2Type<BT_PROD> > ();
  else if (branch == "stage")
    return hub_post_url_temp_internal<Branch2Type<BT_MASTER> > ();
  else return hub_post_url_temp_internal<Branch2Type<BT_DEV> > ();
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return hub_register_url_temp_internal<Branch2Type<BT_PROD> > ();
  else if (branch == "stage")
    return hub_register_url_temp_internal<Branch2Type<BT_MASTER> > ();
  else return hub_register_url_temp_internal<Branch2Type<BT_DEV> > ();
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return hub_user_profile_temp_internal<Branch2Type<BT_PROD> > ();
  else if (branch == "stage")
    return hub_user_profile_temp_internal<Branch2Type<BT_MASTER> > ();
  else return hub_user_profile_temp_internal<Branch2Type<BT_DEV> > ();
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return hub_get_url_temp_internal<Branch2Type<BT_PROD> > ();
  else if (branch == "stage")
    return hub_get_url_temp_internal<Branch2Type<BT_MASTER> > ();
  else return hub_get_url_temp_internal<Branch2Type<BT_DEV> > ();
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return hub_healt_url_temp_internal<Branch2Type<BT_PROD> > ();
  else if (branch == "stage")
    return hub_healt_url_temp_internal<Branch2Type<BT_MASTER> > ();
  else return hub_healt_url_temp_internal<Branch2Type<BT_DEV> > ();
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return hub_kurjun_url_temp_internal<Branch2Type<BT_PROD> > ();
  else if (branch == "stage")
    return hub_kurjun_url_temp_internal<Branch2Type<BT_MASTER> > ();
  else return hub_kurjun_url_temp_internal<Branch2Type<BT_DEV> > ();
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return hub_billing_temp_internal<Branch2Type<BT_PROD> > ();
  else if (branch == "stage")
    return hub_billing_temp_internal<Branch2Type<BT_MASTER> > ();
  else return hub_billing_temp_internal<Branch2Type<BT_DEV> > ();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QStringList& supported_browsers_internal();

#define supported_browsers_internal_def(OS_TYPE, STRING) \
    template <> \
    const QStringList& supported_browsers_internal<Os2Type <OS_TYPE> >() { \
        static QString st(STRING); \
        static QStringList res = st.split(" "); \
        return res; \
    }

supported_browsers_internal_def(OS_WIN, "Chrome Firefox") // add edge, mozilla
supported_browsers_internal_def(OS_LINUX, "Chrome Firefox") // add mozilla
supported_browsers_internal_def(OS_MAC, "Chrome Firefox") // add safari , mozilla

const QStringList& supported_browsers(){
    return supported_browsers_internal<Os2Type <CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////

template<class OS> const QString& chrome_profiles_internal();

template<>
const QString& chrome_profiles_internal<Os2Type<OS_LINUX>>() {
  QStringList paths =
      QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
  static QString path;
  if (!paths.empty()) {
    path = (*paths.begin()).append("/google-chrome/Local State");
  } else {
    path = "empty";
  }
  return path;
}

template<>
const QString& chrome_profiles_internal<Os2Type<OS_MAC>>() {
  QStringList paths =
      QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation);
  static QString path;
  if (!paths.empty()) {
    path = (*paths.begin()).append("/Google/Chrome/Local State");
  } else {
    path = "empty";
  }
  return path;
}

template<>
const QString& chrome_profiles_internal<Os2Type<OS_WIN>>() {
  QStringList paths =
      QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
  static QString path;
  if (!paths.empty()) {
    path = (*paths.begin()).append("\\Google\\Chrome\\User Data\\Local State");
  } else {
    path = "empty";
  }
  return path;
}

const std::pair<QStringList, QStringList>& chrome_profiles(){
  static std::pair<QStringList, QStringList> profiles;
  profiles.first.clear();
  profiles.second.clear();
  QString path = chrome_profiles_internal<Os2Type<CURRENT_OS>>();

  qDebug() << "Got chrome profiles directory:" << path;

  if (path != "empty") {
    QFile jsonFile(path);
    jsonFile.open(QFile::ReadOnly);
    QJsonObject obj = QJsonDocument().fromJson(jsonFile.readAll()).object();

    if (obj["profile"].isObject()) {
      obj = obj["profile"].toObject();

      if (obj["info_cache"].isObject()) {
        obj = obj["info_cache"].toObject();
        QStringList keys = obj.keys();

        for (auto profile_key : keys) {
          if (obj[profile_key].isObject()) {
            QJsonObject cur = obj[profile_key].toObject();
            QString profile_name;
            bool name_set = false;

            if (cur.contains("gaia_given_name")) {
              profile_name = cur["gaia_given_name"].toString();
              name_set = true;
            } else if (cur.contains("name")) {
              profile_name = cur["name"].toString();
              name_set = true;
            }

            if (name_set) {
              profiles.first.append(profile_key);
              profiles.second.append(profile_name);
            }
          }
        }
      }
    }
  }

  qDebug() << "Got profile keys list:" << profiles.first;
  qDebug() << "Got profile names list:" << profiles.second;

  return profiles;
}
////////////////////////////////////////////////////////////////////////////

template<class OS>
const QString firefox_profiles_internal();

template<>
const QString firefox_profiles_internal<Os2Type<OS_LINUX>>() {
  QStringList paths_str = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  return paths_str[0] + "/.mozilla/firefox/profiles.ini";
}

template<>
const QString firefox_profiles_internal<Os2Type<OS_MAC>>() {
  QStringList paths_str = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  return paths_str[0] + "/Library/Application\\\\ Support/Firefox/profiles.ini";
}

template<>
const QString firefox_profiles_internal<Os2Type<OS_WIN>>() {
  QStringList paths_str = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  return paths_str[0] + "\\AppData\\Roaming\\Mozilla\\Firefox\\profiles.ini";
}

const QStringList &firefox_profiles() {
  static QStringList profiles;
  profiles.clear();
  QString ini_path = firefox_profiles_internal<Os2Type<CURRENT_OS>>();
  QFile ini_file(ini_path);

  if (ini_file.open(QIODevice::ReadOnly)) {
    QTextStream stream(&ini_file);

    while (!stream.atEnd()) {
      QString str = stream.readLine();

      if (str.contains("Name=")) {
        str = str.right(str.size() - 5);
        str.replace(QRegularExpression("[ \n\t]"), "");
        profiles << str;
      }
    }
  }
  return profiles;
}

////////////////////////////////////////////////////////////////////////////
template<class BR, class VER> const char* ssdp_rh_search_target_temp_internal();

#define ssdp_rh_search_target_temp_internal_def(BT_TYPE, VERSION, STRING) \
  template<> \
  const char* ssdp_rh_search_target_temp_internal<Branch2Type<BT_TYPE> , Int2Type<VERSION> >() { \
    return STRING; \
  }

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 1, "urn:subutai:management:peer:1")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   1,  "urn:subutai:management:peer:1")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    1,  "urn:subutai:management:peer:1")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 2, "urn:subutai:management:peer:2")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   2,  "urn:subutai:management:peer:2")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    2,  "urn:subutai:management:peer:2")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 3, "urn:subutai:management:peer:3")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   3,  "urn:subutai:management:peer:3")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    3,  "urn:subutai:management:peer:3")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 4, "urn:subutai:management:peer:4")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   4,  "urn:subutai:management:peer:4")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    4,  "urn:subutai:management:peer:4")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 5, "urn:subutai:management:peer:5")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   5,  "urn:subutai:management:peer:5")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    5,  "urn:subutai:management:peer:5")

ssdp_rh_search_target_temp_internal_def(BT_MASTER, 6, "urn:subutai:management:peer:6")
ssdp_rh_search_target_temp_internal_def(BT_PROD,   6,  "urn:subutai:management:peer:6")
ssdp_rh_search_target_temp_internal_def(BT_DEV,    6,  "urn:subutai:management:peer:6")

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
  const QString branch = set_application_branch();
  if (branch == "production")
    return p2p_dht_arg_temp_internal<Branch2Type<BT_PROD> > ();
  else  if (branch == "stage")
    return p2p_dht_arg_temp_internal<Branch2Type<BT_MASTER> > ();
  else
    return p2p_dht_arg_temp_internal<Branch2Type<BT_DEV> > ();
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
default_p2p_path_internal_def(OS_MAC, "/usr/local/bin/p2p")

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
default_x2goclient_path_internal_def(OS_MAC, "/Applications/x2goclient.app/Contents/MacOS/x2goclient")
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return hub_site_temp_internal<Branch2Type<BT_PROD> > ();
  else  if (branch == "stage")
    return hub_site_temp_internal<Branch2Type<BT_MASTER> > ();
  else
    return hub_site_temp_internal<Branch2Type<BT_DEV> > ();
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
which_cmd_internal_def(OS_MAC, "/usr/bin/which")
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

template<class OS> const QString& default_firefox_path_internal();

#define default_firefox_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_firefox_path_internal<Os2Type<OS_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

default_firefox_path_internal_def(OS_LINUX, "/usr/bin/firefox")
default_firefox_path_internal_def(OS_MAC, "/Applications/Firefox.app/Contents/MacOS/Firefox")
default_firefox_path_internal_def(OS_WIN, "C:\\Program Files\\Mozilla Firefox\\firefox.exe")

const QString &
default_firefox_path() {
  return default_firefox_path_internal<Os2Type<CURRENT_OS> >();
}

////////////////////////////////////////////////////////////////////////////
template<class BR> const QString& subutai_command_internal();

#define subutai_command_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& subutai_command_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

subutai_command_internal_def(BT_PROD,   "/usr/bin/subutai")
subutai_command_internal_def(BT_MASTER, "/usr/bin/subutai")
subutai_command_internal_def(BT_DEV,    "/usr/bin/subutai")

const QString &
subutai_command() {
  return subutai_command_internal<Branch2Type<CURRENT_BRANCH> > ();
}
////////////////////////////////////////////////////////////////////////////

template<class BR> const QString& snap_p2p_path_internal();

#define snap_p2p_path_internal_def(BT_TYPE, STRING) \
  template<> \
  const QString& snap_p2p_path_internal<Branch2Type<BT_TYPE> >() { \
    static QString res(STRING); \
    return res; \
  }

snap_p2p_path_internal_def(BT_PROD,   "/usr/bin/p2p")
snap_p2p_path_internal_def(BT_MASTER, "/usr/bin/p2p")
snap_p2p_path_internal_def(BT_DEV,    "/usr/bin/p2p")

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
  const QString branch = set_application_branch();
  if (branch == "production")
    return p2p_package_url_temp_internal<Branch2Type<BT_PROD>, Os2Type<CURRENT_OS> > ();
  else  if (branch == "stage")
    return p2p_package_url_temp_internal<Branch2Type<BT_MASTER>, Os2Type<CURRENT_OS> > ();
  else
    return p2p_package_url_temp_internal<Branch2Type<BT_DEV>, Os2Type<CURRENT_OS> > ();
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
  const QString branch = set_application_branch();
  if (branch == "production")
    return branch_name_str_temp_internal<Branch2Type<BT_PROD>>();
  else if (branch == "stage")
    return branch_name_str_temp_internal<Branch2Type<BT_MASTER>>();
  else
    return branch_name_str_temp_internal<Branch2Type<BT_DEV>>();
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

////////////////////////////////////////////////////////////////////////////
template<class OS> const QString& default_chrome_extensions_path_internal();

#define default_chrome_extensions_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_chrome_extensions_path_internal<Os2Type<OS_TYPE>>() { \
    static QString res(STRING); \
    return res; \
  }

default_chrome_extensions_path_internal_def(OS_WIN, "/AppData/Local/Google/Chrome/User Data/")
default_chrome_extensions_path_internal_def(OS_MAC, "/Library/Application Support/Google/Chrome/")
default_chrome_extensions_path_internal_def(OS_LINUX, "/.config/google-chrome/")

const QString&
default_chrome_extensions_path() {
    return default_chrome_extensions_path_internal< Os2Type<CURRENT_OS> >();
}
///////////////////////////////////////////////////////////////////////////////
template<class OS> const QString& default_firefox_extensions_path_internal();

#define default_firefox_extensions_path_internal_def(OS_TYPE, STRING) \
  template<> \
  const QString& default_firefox_extensions_path_internal<Os2Type<OS_TYPE>>() { \
    static QString res(STRING); \
    return res; \
  }

default_firefox_extensions_path_internal_def(OS_WIN, "")
default_firefox_extensions_path_internal_def(OS_MAC, "")
default_firefox_extensions_path_internal_def(OS_LINUX, "")

const QString&
default_firefox_extensions_path() {
    return default_firefox_extensions_path_internal< Os2Type<CURRENT_OS> >();
}
///////////////////////////////////////////////////////////////////////////////
void current_os_info(std::vector<std::pair<QString, QString> >& v){
    v.clear();
    QString flag, st;
    QStringList output;
    switch (CURRENT_OS) {
    case OS_WIN:
        v.push_back(std::make_pair("TYPE", "Windows"));
        v.push_back(std::make_pair("VERSION:",QSysInfo::productVersion()));
        break;
    case OS_MAC:
        v.push_back(std::make_pair("TYPE", "Mac"));
        v.push_back(std::make_pair("VERSION:", QSysInfo::productVersion()));
        break;
    case OS_LINUX:
        v.push_back(std::make_pair("TYPE", "Linux"));
        output = CSystemCallWrapper::lsb_release();
        for (auto s : output){
            flag = st = "";
            for (int i = 0; i < s.length(); i++)
                if(s[i] == '\t'){
                    flag = st;
                    st = "";
                }
                else st += s[i];
            if(flag == "Codename:")
                v.push_back(std::make_pair("CODE", st));
        }
        break;
    default:
        break;
    }
}


////////////////////////////////////////////////////////////////////////////
const QString& default_default_browser(){
    static QString res("Chrome");
    return res;
}
////////////////////////////////////////////////////////////////////////////
const QString&  default_default_chrome_profile() {
  static QString res("Default");
  QStringList profile_name_keys = chrome_profiles().first;
  if (!profile_name_keys.empty() && !profile_name_keys.contains(res)) {
    res = *profile_name_keys.begin();
  }
  return res;
}
////////////////////////////////////////////////////////////////////////////
const QString& default_default_firefox_profile() {
  static QString res("default");
  QStringList profiles = firefox_profiles();
  if (!profiles.empty() && !profiles.contains(res)) {
    res = *profiles.begin();
  }
  return res;
}
////////////////////////////////////////////////////////////////////////////
const QString& subutai_e2e_id(const QString& current_browser){
  static QString res = "";
  if (current_browser == "Chrome") {
    res = "ffddnlbamkjlbngpekmdpnoccckapcnh";
  } else if (current_browser == "Firefox") {
    res = "jid1-KejrJUY3AaPCkZ";
  }
  return res;
}
////////////////////////////////////////////////////////////////////////////
const QString& set_application_branch(QString branch) {
  static QString application_branch(current_branch_name());
  if (branch == "production" ||
      branch == "stage" ||
      branch == "development") {
    application_branch = branch;
  }
  return application_branch;
}
