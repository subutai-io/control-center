#include "rtm/include/RtmTop.h"
#include "SystemCallWrapper.h"

#include <vector>
#include <iterator>
#include <regex>
#include "ApplicationLog.h"

using namespace realtime_monitoring;

top_uptime
CTopRawParser::parse_uptime(const std::string &str) {
  top_uptime res;

  std::regex ws("\\S+");
  auto wb = std::sregex_iterator(str.begin(), str.end(), ws);
  auto we = std::sregex_iterator();

  for (std::sregex_iterator i = wb; i != we; ++i) {
    std::smatch match = *i;
    std::string match_str = match.str();
    CApplicationLog::Instance()->LogTrace("%s", match_str.c_str());
  }

  return res;
}
////////////////////////////////////////////////////////////////////////////

top_task
CTopRawParser::parse_task(const std::string &str) {
  UNUSED_ARG(str);
  top_task res;
  return res;
}
////////////////////////////////////////////////////////////////////////////

top_cpu
CTopRawParser::parse_cpu(const std::string &str) {
  UNUSED_ARG(str);
  top_cpu res;
  return res;
}
////////////////////////////////////////////////////////////////////////////

top_memory
CTopRawParser::parse_memory(const std::string &str) {
  UNUSED_ARG(str);
  top_memory res;
  return res;
}
////////////////////////////////////////////////////////////////////////////

top_swap
CTopRawParser::parse_swap(const std::string &str) {
  UNUSED_ARG(str);
  top_swap res;
  return res;
}
////////////////////////////////////////////////////////////////////////////

top_process
CTopRawParser::parse_proc(const std::string &str) {
  UNUSED_ARG(str);
  top_process res;
  return res;
}
////////////////////////////////////////////////////////////////////////////

void
CTopRawParser::megatest() {
  std::vector<std::string> lst_out;
  int ec = -1;
  if (CSystemCallWrapper::top(lst_out, ec) == SCWE_SUCCESS) {
    if (lst_out.empty()) return;

    parse_uptime(lst_out[0]);
    parse_task(lst_out[1]);
    parse_cpu(lst_out[2]);
    parse_memory(lst_out[3]);
    parse_swap(lst_out[4]);
  }
}
////////////////////////////////////////////////////////////////////////////
