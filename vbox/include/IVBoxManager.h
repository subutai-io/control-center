#ifndef IVBOXMANAGER_H
#define IVBOXMANAGER_H


#include <stdint.h>
#include <map>
#include <QObject>

#include "VBoxCommons.h"
#include "IVirtualMachine.h"

class CVBoxManagerSingleton;

/*!
 * \brief Abstract class for managing virtual machines.
 * This class is abstract because we have two versions : with COM interface and with XPCOM interface.
 */
class IVBoxManager : public QObject {
  Q_OBJECT

private:
  friend class CVBoxManagerSingleton;

protected:
  IVBoxManager();
  virtual ~IVBoxManager();

  nsresult m_last_error;
  vb_errors_t m_last_vb_error;

  std::map<QString, IVirtualMachine*> m_dct_machines;
  typedef void (IVBoxManager::*pf_event_handle)(IEvent*);
  std::map<uint32_t, pf_event_handle> m_dct_event_handlers;

  /*!
   * \brief Occures when virtual machine's state changed
   */
  virtual void on_machine_state_changed(IEvent* event) = 0;
  /*!
   * \brief Occures when new virtual machine is registered
   */
  virtual void on_machine_registered(IEvent* event) = 0;
  /*!
   * \brief Occures when virtual machine session's state changed
   */
  virtual void on_session_state_changed(IEvent* event) = 0;
  /*!
   * \brief When something happens with virtual machine
   */
  virtual void on_machine_event(IEvent* event) = 0;

  void init_event_handlers(void);

public:
  /*!
   * \brief COM or XPCOM last error code
   */
  nsresult last_error(void) const {return m_last_error;}
  /*!
   * \brief Last vb_errors_t code
   */
  vb_errors_t last_vb_error(void) const {return m_last_vb_error;}

  /*!
   * \brief Virtual machine by ID
   */
  const IVirtualMachine* vm_by_id(const QString& id) const {
    return m_dct_machines.find(id) == m_dct_machines.end() ? NULL : m_dct_machines.at(id);}

  /*!
   * \brief Count of virtual machines
   */
  size_t vm_count() const {return m_dct_machines.size();}

  /*!
   * \brief Dictionary that contains all available virtual machines
   */
  const std::map<QString, IVirtualMachine*>& dct_machines() const {return m_dct_machines;}

  /*!
   * \brief Retrieve list of available virtual machines and add it to m_dct_machines
   */
  virtual int init_machines(void) = 0;

  /*!
   * \brief Launch VM with specified ID
   */
  virtual int launch_vm(const QString& vm_id,
                        vb_launch_mode_t lm = VBML_HEADLESS) = 0;

  /*!
   * \brief Turn off VM with specified ID
   */
  virtual int turn_off(const QString& vm_id,
                       bool save_state = false) = 0;

  /*!
   * \brief Obsolete. Will be removed in next versions of tray
   */
  int launch_process(const QString& vm_id,
                     const char* path,
                     const char* user,
                     const char* password,
                     const char** argv,
                     int argc);

  virtual int pause(const QString& vm_id) = 0;
  virtual int resume(const QString& vm_id) = 0;
  virtual int remove(const QString& vm_id) = 0;
  virtual int add(const QString& vm_id) = 0;
  virtual QString version() = 0;
  virtual void shutdown_com() = 0;

  nsresult handle_event(VBoxEventType_T e_type,
                        IEvent *event);
  void init_com();

signals:
  void vm_add(const QString& vm_id);
  void vm_remove(const QString& vm_id);
  void vm_state_changed(const QString& vm_id);
  void vm_session_state_changed(const QString& vm_id);
  void vm_launch_progress(const QString& vm_id, uint32_t percent);
  void vm_save_state_progress(const QString& vm_id, uint32_t percent);
  void vm_turn_off_progress(const QString& vm_id, uint32_t percent);
};
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief Singleton of IVboxManager.
 */
class CVBoxManagerSingleton {
private:
  class CVBoxManagerDestructor {
  private:
    IVBoxManager* m_instance;
  public:
    CVBoxManagerDestructor() : m_instance(NULL) {}
    ~CVBoxManagerDestructor(void) {if (m_instance) delete m_instance;}
    void Init(IVBoxManager* instance) {m_instance = instance;}
  };

  static IVBoxManager* p_instance;
  static CVBoxManagerDestructor p_destructor;

public:
  static IVBoxManager* Instance();
};
#endif // VBOXMANAGER_H
