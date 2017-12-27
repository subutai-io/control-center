#include <stdio.h>

#include "SsdpController.h"
#include "SettingsManager.h"
#include "Commons.h"
#include "OsBranchConsts.h"

#include "RestWorker.h"
#include "HubController.h"

static const char* ssdp_start_lines[] = {
  "NOTIFY * HTTP/1.1\r\n",
  "M-SEARCH * HTTP/1.1\r\n",
  "HTTP/1.1 200 OK\r\n"
};

static const char* SSDP_HOST_ADDRESS = "239.255.255.250";
static const int   SSDP_PORT = 1900;

void
CSsdpController::init(std::vector<vertex_t> &dma) {
  vertex_t root;
  root.parent = root.link = 0;
  dma.push_back(root);
}
////////////////////////////////////////////////////////

void
CSsdpController::add_string(const std::string& str,
                            std::vector<vertex_t> &dma,
                            size_t si,
                            interested_fields_en ife) {
  int32_t v = 0;
  int32_t sz = (int32_t)dma.size();

  for (size_t i=0; i < str.length(); ++i) {
    int8_t c = str[i];
    if (dma[v].next.find(c) == dma[v].next.end()) {
      vertex_t node;
      node.parent = v;
      node.p_ch = c;
      dma[v].next[c] = sz++;
      dma.push_back(node);
    }
    v = dma[v].next[c];
  }
  dma[v].leaf = true;
  dma[v].si = si;
  dma[v].ife = ife;
}
////////////////////////////////////////////////////////

int32_t
CSsdpController::go(int32_t v, int8_t c, std::vector<vertex_t> &dma) {
  if (dma[v].go.find(c) == dma[v].go.end()) {
    if (dma[v].next.find(c) != dma[v].next.end())
      dma[v].go[c] = dma[v].next[c];
    else
      dma[v].go[c] = v==0 ? 0 : go(get_link(v, dma), c, dma);
  }
  return dma[v].go[c];
}
////////////////////////////////////////////////////////

int32_t
CSsdpController::get_link(int32_t v, std::vector<vertex_t> &dma) {
  if (dma[v].link == vertex_t::NOT_INITIALIZED) {
    if (v == 0 || dma[v].parent == 0)
      dma[v].link = 0;
    else
      dma[v].link = go(get_link(dma[v].parent, dma), dma[v].p_ch, dma);
  }
  return dma[v].link;
}
////////////////////////////////////////////////////////

int32_t
CSsdpController::nearest(int32_t v, std::vector<vertex_t> &dma) {
  if (dma[v].nl != vertex_t::NOT_INITIALIZED) return dma[v].nl;
  int32_t cv = v;
  while ((cv = get_link(cv, dma))) {
    if (!dma[cv].leaf) continue;
    return dma[v].nl = cv;
  }
  return dma[v].nl;
}
////////////////////////////////////////////////////////

static std::vector<std::string> interested_fields_strings[] = {
  {"\r\nLOCATION:", "\r\nlocation:",
   "\r\nNT:", "\r\nnt:",
   "\r\nNTS:", "\r\nnts:",
   "\r\nUSN:", "\r\nusn:"},

  {"\r\nLOCATION:",
   "\r\nlocation:"},

  {"\r\nLOCATION:", "\r\nlocation:",
   "\r\nST:", "\r\nst:",
   "\r\nUSN:", "\r\nusn:"}
};

static std::vector<interested_fields_en> interested_fields[] = {
  {ife_location, ife_location,
   ife_nt, ife_nt,
   ife_nts, ife_nts,
   ife_usn, ife_usn},

  {ife_location, ife_location},

  {ife_location, ife_location,
   ife_st, ife_st,
   ife_usn, ife_usn},
};
////////////////////////////////////////////////////////////////////////////

CSsdpController::CSsdpController(QObject *parent) :
  QObject(parent),
  m_group_address(SSDP_HOST_ADDRESS) {

  std::vector<vertex_t>* dmas[] =
    {&m_ak_notify_dma, &m_ak_search_dma, &m_ak_ok_dma};


  for (int i = 0; i <= smt_ok; ++i) {
    init(*dmas[i]);

    for (size_t fi = 0; fi < interested_fields_strings[i].size(); ++fi) {
      add_string(interested_fields_strings[i][fi], *dmas[i], fi, interested_fields[i][fi]);
    }

    for (int32_t j = 0; j < (int32_t)dmas[i]->size(); ++j) {
      get_link(j, *dmas[i]);
    }
  }

  m_socket = new QUdpSocket(this);
  m_socket->bind(QHostAddress::AnyIPv4, SSDP_PORT, QUdpSocket::ShareAddress);
  m_socket->joinMulticastGroup(m_group_address);
  set_ttl(2);
  send_search();
  connect(m_socket, &QUdpSocket::readyRead,
          this, &CSsdpController::process_pending_datagrams);
}

CSsdpController::~CSsdpController() {
}
////////////////////////////////////////////////////////////////////////////

void
CSsdpController::set_ttl(int ttl) {
  m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, ttl);
}
////////////////////////////////////////////////////////////////////////////

void
CSsdpController::send_datagram(const QByteArray &dtgr) {
  m_socket->writeDatagram(dtgr.data(), dtgr.size(), m_group_address, SSDP_PORT);
}

std::map<interested_fields_en, std::string>
CSsdpController::parse_ssdp_datagram(const QByteArray &dtgr,
                                     std::vector<CSsdpController::vertex_t> &dma) {
  int cs = 0; //current state
  std::map<interested_fields_en, std::string> dct_packet;

  for (int i = 0; i < dtgr.size(); ++i) {
    char cc = dtgr.data()[i];

    if (dma[cs].leaf ||
        nearest(cs, dma) != vertex_t::NOT_INITIALIZED) {

      int k = i;
      for (; i < dtgr.size()-1; ++i) {
        if (dtgr.data()[i] == '\r' && dtgr.data()[i+1] == '\n') {
          --i; break;
        }
      }

      for (; k < i; ++k) {
        if (dtgr.data()[k] != ' ') break;
      }

      dct_packet[dma[cs].ife] = std::string(&dtgr.data()[k], i - k + 1);
    }
    cs = go(cs, cc, dma);
  }
  return dct_packet;
}

void
CSsdpController::handle_ssdp_notify(const QByteArray &dtgr) {
  std::map<interested_fields_en, std::string> dct_packet =
      parse_ssdp_datagram(dtgr, m_ak_notify_dma);
  if (dct_packet.find(ife_usn) == dct_packet.end())
    return;
}
////////////////////////////////////////////////////////////////////////////

void
CSsdpController::handle_ssdp_search(const QByteArray &dtgr) {
  UNUSED_ARG(dtgr);
  /*do nothing*/
}
////////////////////////////////////////////////////////////////////////////

void
CSsdpController::hanvle_ssdp_ok(const QByteArray &dtgr) {
  std::map<interested_fields_en, std::string> dct_packet =
      parse_ssdp_datagram(dtgr, m_ak_ok_dma);

  if (dct_packet.find(ife_st) == dct_packet.end()) return;
  bool valid_st = false;
  for (int i = 0; ssdp_rh_search_target_arr()[i]; ++i) {
    if (dct_packet[ife_st] == std::string(ssdp_rh_search_target_arr()[i])) {
      valid_st = true;
      break;
    }
  }

  if (!valid_st) return;
  if (dct_packet.find(ife_location) == dct_packet.end()) return;
  if (dct_packet.find(ife_usn) == dct_packet.end()) return;

  emit found_device(
        QString::fromStdString(dct_packet[ife_usn]),
        QString::fromStdString(dct_packet[ife_location]));
}
////////////////////////////////////////////////////////////////////////////

void
CSsdpController::handle_ssdp_packet(const QByteArray &dtgr) {
  void (CSsdpController::*ssdp_handlers[])(const QByteArray &dtgr) =
  {&CSsdpController::handle_ssdp_notify, &CSsdpController::handle_ssdp_search, &CSsdpController::hanvle_ssdp_ok};

  bool valid = false;
  int li ; //line index
  for (li = 0; li <= smt_ok; ++li) {
    if ((valid = dtgr.startsWith(ssdp_start_lines[li])))
      break;
  }
  if (!valid) return; //ignore unknown messages
  (this->*ssdp_handlers[li])(dtgr);
}
////////////////////////////////////////////////////////////////////////////

void
CSsdpController::send_search() {
  static const int send_buff_size = 0xff;
  static const char* search_format =
      "%sHOST: %s:%d\r\nST: %s\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\n\r\n";
  char buffer[send_buff_size] = {0}; //let this buffer located on stack

  for (int i = 0; ssdp_rh_search_target_arr()[i]; ++i) {
    int res_size = sprintf(buffer, search_format, ssdp_start_lines[smt_search],
                           SSDP_HOST_ADDRESS, SSDP_PORT, ssdp_rh_search_target_arr()[i]);
    send_datagram(QByteArray(buffer, res_size));
  }
}
////////////////////////////////////////////////////////////////////////////

void
CSsdpController::process_pending_datagrams() {
  while (m_socket->hasPendingDatagrams()) {
    QByteArray datagram;
    datagram.resize(m_socket->pendingDatagramSize());
    m_socket->readDatagram(datagram.data(), datagram.size());
    handle_ssdp_packet(datagram);
  }
}
////////////////////////////////////////////////////////////////////////////
