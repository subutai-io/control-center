#include "rtm/include/RtmController.h"
#include "SystemCallWrapper.h"
#include "libssh2/include/LibsshController.h"
#include "ApplicationLog.h"

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
  int rc = CLibsshController::run_ssh_command("127.0.0.1",
                                              22,
                                              "lezh1k",
                                              "Ajhthjkt@1234",
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
  proc_load_avg_t sys1 = m_rpp_syscall->load_average();
  proc_load_avg_t sys2 = m_rpp_libssh2->load_average();

  proc_meminfo_t mem1  = m_rpp_syscall->meminfo();
  proc_meminfo_t mem2  = m_rpp_libssh2->meminfo();

  std::vector<proc_net_dev_t> ni1 = m_rpp_syscall->network_info();
  std::vector<proc_net_dev_t> ni2 = m_rpp_libssh2->network_info();

  proc_uptime_t up1 = m_rpp_syscall->uptime();
  proc_uptime_t up2 = m_rpp_libssh2->uptime();

  CApplicationLog::Instance()->LogInfo("Abircheg");
}
////////////////////////////////////////////////////////////////////////////
