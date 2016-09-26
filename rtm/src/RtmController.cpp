#include "rtm/include/RtmController.h"
#include "SystemCallWrapper.h"
#include "libssh2/include/LibsshController.h"
#include "ApplicationLog.h"
#include "SettingsManager.h"

std::vector<std::string>
rtm::CRtmController::get_output_by_syscall(const char *cmd,
                                           bool *success) {
  std::vector<std::string> lst_out;
  int ec = -1;
  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::ssystem_th(cmd, lst_out, ec, true);
  *success = scwe == SCWE_SUCCESS;
  return lst_out;
}
////////////////////////////////////////////////////////////////////////////

std::vector<std::string>
rtm::CRtmController::get_output_by_libssh2(const char *cmd,
                                           bool *success) {
  std::vector<std::string> lst_out;
  int rc = CLibsshController::run_ssh_command(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                                              CSettingsManager::Instance().rh_port(),
                                              CSettingsManager::Instance().rh_user().toStdString().c_str(),
                                              CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                                              cmd,
                                              10,
                                              lst_out);
  *success = rc == 0;
  return lst_out;
}
////////////////////////////////////////////////////////////////////////////

rtm::CRtmController::CRtmController() {
  m_rpp_syscall = new CRtmProcParser(CRtmController::get_output_by_syscall);
  m_rpp_libssh2 = new CRtmProcParser(CRtmController::get_output_by_libssh2);
}

rtm::CRtmController::~CRtmController() {

}
////////////////////////////////////////////////////////////////////////////

rtm::CRtmController*
rtm::CRtmController::Instance() {
  static CRtmController inst;
  return &inst;
}
////////////////////////////////////////////////////////////////////////////

void
rtm::CRtmController::Megatest() {
  bool succes;
  proc_load_avg_t sys1 = m_rpp_syscall->load_average(succes);
  proc_load_avg_t sys2 = m_rpp_libssh2->load_average(succes);

  proc_meminfo_t mem1  = m_rpp_syscall->meminfo(succes);
  proc_meminfo_t mem2  = m_rpp_libssh2->meminfo(succes);

  std::vector<proc_net_dev_t> ni1 = m_rpp_syscall->network_info(succes);
  std::vector<proc_net_dev_t> ni2 = m_rpp_libssh2->network_info(succes);

  proc_uptime_t up1 = m_rpp_syscall->uptime(succes);
  proc_uptime_t up2 = m_rpp_libssh2->uptime(succes);

  CApplicationLog::Instance()->LogInfo("Abircheg");
}
////////////////////////////////////////////////////////////////////////////
