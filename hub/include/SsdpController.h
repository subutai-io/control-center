#ifndef SSDPRECEIVER_H
#define SSDPRECEIVER_H

#include <QObject>
#include <QtNetwork>
#include <vector>
#include <stdint.h>
#include <map>
#include <string>

enum ssdp_msg_type {
  smt_notify = 0,
  smt_search,
  smt_ok
};

enum interested_fields_en {
  ife_location = 0,
  ife_nt,
  ife_nts,
  ife_usn,
  ife_st,
  ife_nothing
};
////////////////////////////////////////////////////////////////////////////

/**
 * @brief The CSsdpController class sends ssdp search request and handles answers
 * with Aho-Korasic algorithm (https://en.wikipedia.org/wiki/Aho–Corasick_algorithm)
 */
class CSsdpController : public QObject {
  Q_OBJECT
public:
  static CSsdpController* Instance() {
    static CSsdpController instance;
    return &instance;
  }  
  void search() {send_search();}

private:  

  typedef std::map<char, int32_t> dct_char_state;
  struct vertex_t {
    static const int32_t NOT_INITIALIZED = -1;
    bool leaf;
    int32_t parent;
    int32_t p_ch;
    int32_t link;
    int32_t nl; //nearest leaf that we can arrive via links.
    size_t si; //string index
    interested_fields_en ife;

    dct_char_state next;
    dct_char_state go;

    vertex_t() :
      leaf(false),
      parent(NOT_INITIALIZED),
      p_ch(NOT_INITIALIZED),
      link(NOT_INITIALIZED),
      nl(NOT_INITIALIZED),
      ife(ife_nothing) {
    }

    ~vertex_t() {}
  };
  ////////////////////////////////////////////////////////


  std::vector<vertex_t> m_ak_notify_dma;
  std::vector<vertex_t> m_ak_search_dma;
  std::vector<vertex_t> m_ak_ok_dma;

  void init(std::vector<vertex_t>& dma);
  void add_string(const std::string& str, std::vector<vertex_t>& dma, size_t si, interested_fields_en ife);
  int32_t go(int32_t v, int8_t c, std::vector<vertex_t>& dma);
  int32_t get_link(int32_t v, std::vector<vertex_t>& dma);
  int32_t nearest(int32_t v, std::vector<vertex_t>& dma);

  CSsdpController(QObject* parent = nullptr);
  virtual ~CSsdpController();

  QHostAddress m_group_address;
  QUdpSocket* m_socket;
  std::vector<QString> m_lst_known_devs;

  void send_search();
  void set_ttl(int ttl);
  void send_datagram(const QByteArray& dtgr);

  std::map<interested_fields_en, std::string> parse_ssdp_datagram(const QByteArray& dtgr,
                                                             std::vector<vertex_t>& dma);
  void handle_ssdp_notify(const QByteArray &dtgr);
  void handle_ssdp_search(const QByteArray &dtgr);
  void hanvle_ssdp_ok(const QByteArray &dtgr);
  void handle_ssdp_packet(const QByteArray &dtgr);

private slots:
  void process_pending_datagrams();

signals:
  void found_device(QString uid, QString location);
};

#endif // SSDPRECEIVER_H
