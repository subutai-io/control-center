#ifndef SYSTEMCALLWRAPPER_H
#define SYSTEMCALLWRAPPER_H

#include <stdint.h>
#include <QObject>
#include <QString>
#include <string>
#include <vector>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>
#include "VagrantProvider.h"

//give type for restart p2p
enum restart_p2p_type{
    UPDATED_P2P=0, //when p2p updated, stop and start
    STOPPED_P2P, //when p2p is stopped, start
    STARTED_P2P //when p2p is started, stop
};

// if you are going to change this - change method
// CSystemCallWrapper::scwe_error_to_str(system_call_wrapper_error_t err)
enum system_call_wrapper_error_t {
  /*common errors*/
  SCWE_SUCCESS = 0,
  SCWE_SHELL_ERROR,
  SCWE_PIPE,
  SCWE_SET_HANDLE_INFO,
  SCWE_CREATE_PROCESS,

  /*p2p errors*/
  SCWE_P2P_IS_NOT_RUNNABLE,
  SCWE_CANT_JOIN_SWARM,
  SCWE_CONTAINER_IS_NOT_READY,
  SCWE_CANT_SEND_HANDSHAKE,

  /*ssh errors*/
  SCWE_SSH_LAUNCH_FAILED,
  SCWE_CANT_GET_RH_IP,
  SCWE_CANT_GENERATE_SSH_KEY,

  /*other errors*/
  SCWE_TIMEOUT,
  SCWE_WHICH_CALL_FAILED,
  SCWE_PROCESS_CRASHED,
  SCWE_LAST,
  SCWE_PERMISSION_DENIED,
  SCWE_WRONG_FILE_NAME,
  SCWE_DIR_DOESNT_EXIST,
  SCWE_DIR_EXISTS,
  SCWE_COMMAND_FAILED,
  SCWE_WRONG_TEMP_PATH
};
////////////////////////////////////////////////////////////////////////////

struct system_call_res_t {
  system_call_wrapper_error_t res;
  QStringList out;
  int exit_code;
};

struct system_call_wrapper_install_t {
  system_call_wrapper_error_t res;
  QString version;
};
////////////////////////////////////////////////////////////////////////////

enum restart_service_error_t { RSE_SUCCESS, RSE_MANUAL };
////////////////////////////////////////////////////////////////////////////

static QMutex installer_is_busy;
static QMutex vagrant_is_busy;
static QMutex p2p_is_busy;

////////////////////////////////////////////////////////////////////////////
class CSystemCallThreadWrapper : public QObject {
  Q_OBJECT
 private:
  system_call_res_t m_result;
  QString m_command;
  QStringList m_args;
  bool m_read_output;

 public:
  CSystemCallThreadWrapper(QObject *parent = 0)
      : QObject(parent), m_command(""), m_read_output(true) {}

  CSystemCallThreadWrapper(const QString &command, const QStringList &args,
                           bool arg_read_output, QObject *parent = 0)
      : QObject(parent),
        m_command(command),
        m_args(args),
        m_read_output(arg_read_output) {}

  system_call_res_t result() const { return m_result; }
  bool read_output() const { return m_read_output; }
  void abort() { emit finished(); }

 signals:
  void finished();
};
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief This class contains methods and functions for receiving output of
 * system calls like "ls -la", "p2p version" etc.
 */
class CSystemCallWrapper {
  friend class CSystemCallThreadWrapper;

 private:
  static system_call_wrapper_error_t run_libssh2_command(
      const char *host, uint16_t port, const char *user, const char *pass,
      const char *cmd, int &exit_code, std::vector<std::string> &lst_output);

 public:
  static system_call_res_t ssystem_th(const QString &cmd,
                                      QStringList &args, bool read_output,
                                      bool log,
                                      unsigned long timeout_msec = ULONG_MAX);

  static system_call_res_t ssystem(const QString &cmd, QStringList &args,
                                   bool read_out, bool log,
                                   unsigned long timeout_msec = 30000);

  static system_call_res_t ssystem_f(QString cmd, QStringList arg,
                                     bool read_out, bool log,
                                     unsigned long timeout_sec = 30000);
  // runs bash script
  static system_call_res_t run_script(const QString& file_name, const QByteArray& script);

  static bool is_in_swarm(const QString &hash);
  static std::vector<QString> p2p_show();
  static std::vector<std::pair<QString, QString>> p2p_show_interfaces();

  static std::pair<system_call_wrapper_error_t, QStringList> remove_file(const QString &file_path);

  static system_call_wrapper_error_t create_folder(const QString &dir, const QString &name);

  static std::pair<system_call_wrapper_error_t, QStringList> send_command(
                                                  const QString &remote_user,
                                                  const QString &ip,
                                                  const QString &port,
                                                  const QString &key,
                                                  const QString &commands);

  static std::pair<system_call_wrapper_error_t, QStringList>
                                                 upload_file (
                                                 const QString &remote_user,
                                                 const QString &ip,
                                                 std::pair <QString, QString> ssh_info,
                                                 const QString &destination,
                                                 const QString &file_path
                                                 );

  static std::pair<system_call_wrapper_error_t, QStringList>
                                                    download_file (
                                                    const QString &remote_user,
                                                    const QString &ip,
                                                    std::pair <QString, QString> ssh_info,
                                                    const QString &local_destination,
                                                    const QString &remote_file_path
                                                    );

  static system_call_wrapper_error_t join_to_p2p_swarm(const QString &hash,
                                                       const QString &key,
                                                       const QString &ip,
                                                       int swarm_base_interface_id);

  static system_call_wrapper_error_t leave_p2p_swarm(const QString &hash);
  static system_call_wrapper_error_t restart_p2p_service(int *res_code, restart_p2p_type type);

  static system_call_wrapper_error_t check_container_state(const QString &hash,
                                                           const QString &ip);

  static system_call_wrapper_error_t run_sshkey_in_terminal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &key);

  static system_call_wrapper_error_t vagrant_command_terminal(const QString &dir,
                                                              const QString &command,
                                                              const QString &name);
  static system_call_wrapper_error_t vagrant_destroy(const QString &dir);
  static system_call_wrapper_error_t vagrant_init(const QString &dir, const QString &box);
  static system_call_wrapper_error_t vagrant_box_update(const QString &box, const QString &provider);
  static system_call_wrapper_install_t vagrant_box_remove(const QString &box, const QString &provider);
  static QString vagrant_status(const QString &dir);
  static QString vagrant_ip(const QString &dir);
  static QString vagrant_port(const QString &dir);
  static system_call_wrapper_error_t vagrant_update_peeros(const QString &port, const QString &peer_name);
  static std::pair<QStringList, system_call_res_t> vagrant_update_information();
  static QString get_virtualbox_vm_storage();
  static system_call_wrapper_error_t set_virtualbox_vm_storage(const QString &dir);

  static bool check_peer_management_components();

  static void vagrant_plugins_list(std::vector<std::pair<QString, QString> > &plugins);

  static std::pair<system_call_wrapper_error_t, QStringList> vagrant_up(const QString &dir);

  static system_call_wrapper_error_t vagrant_halt(const QString &dir);

  static system_call_wrapper_error_t vagrant_reload(const QString &dir);

  static system_call_wrapper_error_t vagrant_latest_box_version(const QString &box,
                                                                const QString &provider,
                                                                QString& version);

  static system_call_wrapper_error_t vagrant_add_box(const QString &box,
                                                     const QString &provider,
                                                     const QString &box_dir);

  static system_call_wrapper_error_t give_write_permissions(const QString &dir);

  static QStringList list_interfaces();

  static QStringList virtualbox_interfaces();

  static QStringList libvirt_interfaces();

  static QStringList hyperv_interfaces();

  static QStringList parallels_interfaces();

  static system_call_wrapper_install_t install_p2p(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_p2p(const QString &dir, const QString &file_name);
  static system_call_wrapper_error_t update_p2p_linux(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t update_p2p(const QString &dir, const QString &file_name);

  static system_call_wrapper_install_t install_x2go(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_x2go();

  static system_call_wrapper_install_t install_vagrant(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_vagrant(const QString &dir, const QString &file_name);

  static system_call_wrapper_install_t install_oracle_virtualbox(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_oracle_virtualbox(const QString &dir, const QString &file_name);

  static system_call_wrapper_install_t install_vmware(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_vmware(const QString &dir, const QString &file_name);

  static system_call_wrapper_install_t install_hyperv();
  static system_call_wrapper_install_t uninstall_hyperv();

  static system_call_wrapper_install_t install_vmware_utility(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_vmware_utility(const QString &dir, const QString &file_name);

  static system_call_wrapper_install_t install_chrome(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_chrome(const QString &dir, const QString &file_name);

  static system_call_wrapper_install_t install_firefox(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_firefox(const QString &dir, const QString &file_name);

  static system_call_wrapper_install_t install_e2e(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_e2e();

  static system_call_wrapper_error_t install_e2e_chrome();
  static system_call_wrapper_error_t uninstall_e2e_chrome();

  static system_call_wrapper_error_t install_e2e_firefox(const QString &dir, const QString &file_name);
  static system_call_wrapper_error_t uninstall_e2e_firefox();

  static system_call_wrapper_error_t install_e2e_safari(const QString &dir, const QString &file_name);
  static system_call_wrapper_error_t uninstall_e2e_safari();

  static system_call_wrapper_install_t install_vagrant_subutai();

  static system_call_wrapper_install_t install_vagrant_vbguest();

  static system_call_wrapper_install_t install_subutai_box(const QString &dir, const QString &file_name);

  static system_call_wrapper_install_t install_xquartz(const QString &dir, const QString &file_name);
  static system_call_wrapper_install_t uninstall_xquartz();

  static system_call_wrapper_install_t install_vagrant_libvirt();
  static system_call_wrapper_install_t install_kvm();
  static system_call_wrapper_install_t uninstall_kvm();

  static void run_linux_script(QStringList args);

  static system_call_wrapper_error_t install_libssl();

  static QStringList lsb_release();

  static bool chrome_last_session();

  static bool firefox_last_session();

  static bool safari_last_session();

 // static system_call_res_t chrome_open_link();

  static system_call_wrapper_error_t run_sshpass_in_terminal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &pass);

  static system_call_wrapper_error_t run_x2goclient_session(const QString &session_id);


  static system_call_wrapper_error_t run_x2go(QString remote_ip,
                                              QString remote_port,
                                              QString remote_username);

  static system_call_wrapper_error_t generate_ssh_key(const QString &comment,
                                                      const QString &file_path);

  static system_call_wrapper_error_t remove_ssh_key(const QString &key_name);

  static system_call_wrapper_error_t is_rh_update_available(bool &available);
  static system_call_wrapper_error_t is_rh_management_update_available(
      bool &available);

  static system_call_wrapper_error_t is_peer_available(
      const QString &peer_fingerprint, int* exit_code);

  static system_call_wrapper_error_t run_rh_updater(const char *host,
                                                    uint16_t port,
                                                    const char *user,
                                                    const char *pass,
                                                    int &exit_code);

  static system_call_wrapper_error_t run_rh_management_updater(const char *host,
                                                               uint16_t port,
                                                               const char *user,
                                                               const char *pass,
                                                               int &exit_code);

  static system_call_wrapper_error_t get_rh_ip_via_libssh2(
      const char *host, uint16_t port, const char *user, const char *pass,
      int &exit_code, std::string &ip);

  static QString rh_version();
  static QString rhm_version();

  static system_call_wrapper_error_t p2p_version(QString &version);
  static system_call_wrapper_error_t p2p_status(QString &status);
  static system_call_wrapper_error_t x2go_version(QString &version);
  static system_call_wrapper_error_t vagrant_version(QString &version);
  static system_call_wrapper_error_t oracle_virtualbox_version(QString &version);
  static system_call_wrapper_error_t vmware_version(QString &version);
  static system_call_wrapper_error_t hyperv_version(QString &version);
  static system_call_wrapper_error_t vmware_utility_version(QString &version);
  static system_call_wrapper_error_t subutai_e2e_version(QString &version);
  static system_call_wrapper_error_t vagrant_plugin_version(QString &version, QString plugin);
  static system_call_wrapper_error_t subutai_e2e_chrome_version(QString &version);
  static system_call_wrapper_error_t subutai_e2e_firefox_version(QString &version);
  static system_call_wrapper_error_t subutai_e2e_safari_version(QString &version);
  static system_call_wrapper_error_t vagrant_subutai_version(QString &version);
  static system_call_wrapper_error_t vagrant_vbguest_version(QString &version);
  static system_call_wrapper_error_t xquartz_version(QString &version);
  static system_call_wrapper_error_t kvm_version(QString& version);
  static int versionCompare(std::string v1, std::string v2);
  static bool p2p_daemon_check();
  static bool x2goclient_check();


  static system_call_wrapper_error_t which(const QString &prog, QString &path);
  static system_call_wrapper_error_t open(const QString &prog);

  static bool is_desktop_peer();
  static system_call_wrapper_error_t local_containers_list(QStringList &list);

  static system_call_wrapper_error_t chrome_version(QString &version);
  static system_call_wrapper_error_t firefox_version(QString &version);
  static system_call_wrapper_error_t edge_version(QString &version);
  static system_call_wrapper_error_t safari_version(QString &version);
  static const QString &virtual_box_version();
  static const QString &scwe_error_to_str(system_call_wrapper_error_t err);

  static bool set_application_autostart(bool start);
  static bool application_autostart();

  static system_call_wrapper_install_t vagrant_plugin(const QString& name,
                                                    const QString& command);

  struct container_ip_and_port {
    QString ip;
    QString port;
    bool use_p2p;
  };
  static container_ip_and_port container_ip_from_ifconfig_analog(
      const QString &port, const QString &cont_ip, const QString &rh_ip);
  static bool is_host_reachable(const QString &host);
  // varios system call wrappers for iupdater components
  static system_call_wrapper_error_t tray_post_update();
  static system_call_wrapper_error_t p2p_post_update();
};
/*
CProcessHandler saves running processes
When CC is about to quit , it terminates them
*/
class CProcessHandler : public QObject{
    Q_OBJECT
public:
    CProcessHandler(QObject *parent = nullptr) : QObject (parent), m_hash_counter(0) {}
    ~CProcessHandler() { m_proc_table.clear(); }
    static CProcessHandler *Instance(){
        static CProcessHandler inst;
        return &inst;
    }
    int start_proc(QProcess &proc);
    void end_proc(const int &hash);
    void clear_proc();
    int sum_proc();
    int generate_hash();
private:
    std::map <int, QProcess*> m_proc_table;
    int m_hash_counter;
    QMutex m_proc_mutex; // use when updating proc table
};

#endif  // SYSTEMCALLWRAPPER_H
