/****************************************************************************
** Meta object code from reading C++ file 'TrayControlWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/TrayControlWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrayControlWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TrayControlWindow_t {
    QByteArrayData data[68];
    char stringdata0[1140];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TrayControlWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TrayControlWindow_t qt_meta_stringdata_TrayControlWindow = {
    {
QT_MOC_LITERAL(0, 0, 17), // "TrayControlWindow"
QT_MOC_LITERAL(1, 18, 10), // "show_about"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 20), // "show_settings_dialog"
QT_MOC_LITERAL(4, 51, 10), // "launch_Hub"
QT_MOC_LITERAL(5, 62, 9), // "launch_ss"
QT_MOC_LITERAL(6, 72, 18), // "show_create_dialog"
QT_MOC_LITERAL(7, 91, 28), // "show_notifications_triggered"
QT_MOC_LITERAL(8, 120, 10), // "launch_p2p"
QT_MOC_LITERAL(9, 131, 23), // "launch_p2p_installation"
QT_MOC_LITERAL(10, 155, 13), // "dialog_closed"
QT_MOC_LITERAL(11, 169, 6), // "unused"
QT_MOC_LITERAL(12, 176, 16), // "application_quit"
QT_MOC_LITERAL(13, 193, 21), // "notification_received"
QT_MOC_LITERAL(14, 215, 43), // "CNotificationObserver::notifi..."
QT_MOC_LITERAL(15, 259, 5), // "level"
QT_MOC_LITERAL(16, 265, 3), // "msg"
QT_MOC_LITERAL(17, 269, 41), // "DlgNotification::NOTIFICATION..."
QT_MOC_LITERAL(18, 311, 11), // "action_type"
QT_MOC_LITERAL(19, 323, 6), // "logout"
QT_MOC_LITERAL(20, 330, 13), // "login_success"
QT_MOC_LITERAL(21, 344, 23), // "environments_updated_sl"
QT_MOC_LITERAL(22, 368, 2), // "rr"
QT_MOC_LITERAL(23, 371, 18), // "balance_updated_sl"
QT_MOC_LITERAL(24, 390, 19), // "my_peers_updated_sl"
QT_MOC_LITERAL(25, 410, 16), // "got_peer_info_sl"
QT_MOC_LITERAL(26, 427, 4), // "type"
QT_MOC_LITERAL(27, 432, 4), // "name"
QT_MOC_LITERAL(28, 437, 3), // "dir"
QT_MOC_LITERAL(29, 441, 6), // "output"
QT_MOC_LITERAL(30, 448, 26), // "machine_peers_upd_finished"
QT_MOC_LITERAL(31, 475, 15), // "peer_deleted_sl"
QT_MOC_LITERAL(32, 491, 9), // "peer_name"
QT_MOC_LITERAL(33, 501, 26), // "peer_under_modification_sl"
QT_MOC_LITERAL(34, 528, 16), // "peer_poweroff_sl"
QT_MOC_LITERAL(35, 545, 25), // "my_peer_button_pressed_sl"
QT_MOC_LITERAL(36, 571, 21), // "const my_peer_button*"
QT_MOC_LITERAL(37, 593, 9), // "peer_info"
QT_MOC_LITERAL(38, 603, 27), // "got_ss_console_readiness_sl"
QT_MOC_LITERAL(39, 631, 8), // "is_ready"
QT_MOC_LITERAL(40, 640, 3), // "err"
QT_MOC_LITERAL(41, 644, 19), // "ssh_to_rh_triggered"
QT_MOC_LITERAL(42, 664, 16), // "peer_fingerprint"
QT_MOC_LITERAL(43, 681, 21), // "ssh_to_rh_finished_sl"
QT_MOC_LITERAL(44, 703, 27), // "system_call_wrapper_error_t"
QT_MOC_LITERAL(45, 731, 3), // "res"
QT_MOC_LITERAL(46, 735, 16), // "libssh_exit_code"
QT_MOC_LITERAL(47, 752, 29), // "upload_to_container_triggered"
QT_MOC_LITERAL(48, 782, 12), // "CEnvironment"
QT_MOC_LITERAL(49, 795, 3), // "env"
QT_MOC_LITERAL(50, 799, 13), // "CHubContainer"
QT_MOC_LITERAL(51, 813, 4), // "cont"
QT_MOC_LITERAL(52, 818, 26), // "ssh_to_container_triggered"
QT_MOC_LITERAL(53, 845, 30), // "desktop_to_container_triggered"
QT_MOC_LITERAL(54, 876, 26), // "ssh_key_generate_triggered"
QT_MOC_LITERAL(55, 903, 25), // "ssh_to_container_finished"
QT_MOC_LITERAL(56, 929, 6), // "result"
QT_MOC_LITERAL(57, 936, 29), // "desktop_to_container_finished"
QT_MOC_LITERAL(58, 966, 16), // "update_available"
QT_MOC_LITERAL(59, 983, 7), // "file_id"
QT_MOC_LITERAL(60, 991, 15), // "update_finished"
QT_MOC_LITERAL(61, 1007, 7), // "success"
QT_MOC_LITERAL(62, 1015, 20), // "update_p2p_status_sl"
QT_MOC_LITERAL(63, 1036, 29), // "P2PStatus_checker::P2P_STATUS"
QT_MOC_LITERAL(64, 1066, 6), // "status"
QT_MOC_LITERAL(65, 1073, 25), // "tray_icon_is_activated_sl"
QT_MOC_LITERAL(66, 1099, 33), // "QSystemTrayIcon::ActivationRe..."
QT_MOC_LITERAL(67, 1133, 6) // "reason"

    },
    "TrayControlWindow\0show_about\0\0"
    "show_settings_dialog\0launch_Hub\0"
    "launch_ss\0show_create_dialog\0"
    "show_notifications_triggered\0launch_p2p\0"
    "launch_p2p_installation\0dialog_closed\0"
    "unused\0application_quit\0notification_received\0"
    "CNotificationObserver::notification_level_t\0"
    "level\0msg\0DlgNotification::NOTIFICATION_ACTION_TYPE\0"
    "action_type\0logout\0login_success\0"
    "environments_updated_sl\0rr\0"
    "balance_updated_sl\0my_peers_updated_sl\0"
    "got_peer_info_sl\0type\0name\0dir\0output\0"
    "machine_peers_upd_finished\0peer_deleted_sl\0"
    "peer_name\0peer_under_modification_sl\0"
    "peer_poweroff_sl\0my_peer_button_pressed_sl\0"
    "const my_peer_button*\0peer_info\0"
    "got_ss_console_readiness_sl\0is_ready\0"
    "err\0ssh_to_rh_triggered\0peer_fingerprint\0"
    "ssh_to_rh_finished_sl\0system_call_wrapper_error_t\0"
    "res\0libssh_exit_code\0upload_to_container_triggered\0"
    "CEnvironment\0env\0CHubContainer\0cont\0"
    "ssh_to_container_triggered\0"
    "desktop_to_container_triggered\0"
    "ssh_key_generate_triggered\0"
    "ssh_to_container_finished\0result\0"
    "desktop_to_container_finished\0"
    "update_available\0file_id\0update_finished\0"
    "success\0update_p2p_status_sl\0"
    "P2PStatus_checker::P2P_STATUS\0status\0"
    "tray_icon_is_activated_sl\0"
    "QSystemTrayIcon::ActivationReason\0"
    "reason"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TrayControlWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      35,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  189,    2, 0x0a /* Public */,
       3,    0,  190,    2, 0x0a /* Public */,
       4,    0,  191,    2, 0x0a /* Public */,
       5,    0,  192,    2, 0x0a /* Public */,
       6,    0,  193,    2, 0x0a /* Public */,
       7,    0,  194,    2, 0x0a /* Public */,
       8,    0,  195,    2, 0x0a /* Public */,
       9,    0,  196,    2, 0x0a /* Public */,
      10,    1,  197,    2, 0x08 /* Private */,
      12,    0,  200,    2, 0x08 /* Private */,
      13,    3,  201,    2, 0x08 /* Private */,
      19,    0,  208,    2, 0x08 /* Private */,
      20,    0,  209,    2, 0x08 /* Private */,
      21,    1,  210,    2, 0x08 /* Private */,
      23,    0,  213,    2, 0x08 /* Private */,
      24,    0,  214,    2, 0x08 /* Private */,
      25,    4,  215,    2, 0x08 /* Private */,
      30,    0,  224,    2, 0x08 /* Private */,
      31,    1,  225,    2, 0x08 /* Private */,
      33,    1,  228,    2, 0x08 /* Private */,
      34,    1,  231,    2, 0x08 /* Private */,
      35,    1,  234,    2, 0x08 /* Private */,
      38,    2,  237,    2, 0x08 /* Private */,
      41,    1,  242,    2, 0x08 /* Private */,
      43,    3,  245,    2, 0x08 /* Private */,
      47,    2,  252,    2, 0x08 /* Private */,
      52,    2,  257,    2, 0x08 /* Private */,
      53,    2,  262,    2, 0x08 /* Private */,
      54,    0,  267,    2, 0x08 /* Private */,
      55,    3,  268,    2, 0x08 /* Private */,
      57,    3,  275,    2, 0x08 /* Private */,
      58,    1,  282,    2, 0x08 /* Private */,
      60,    2,  285,    2, 0x08 /* Private */,
      62,    1,  290,    2, 0x08 /* Private */,
      65,    1,  293,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 14, QMetaType::QString, 0x80000000 | 17,   15,   16,   18,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QString, QMetaType::QString, QMetaType::QString,   26,   27,   28,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void, 0x80000000 | 36,   37,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   39,   40,
    QMetaType::Void, QMetaType::QString,   42,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 44, QMetaType::Int,   42,   45,   46,
    QMetaType::Void, 0x80000000 | 48, 0x80000000 | 50,   49,   51,
    QMetaType::Void, 0x80000000 | 48, 0x80000000 | 50,   49,   51,
    QMetaType::Void, 0x80000000 | 48, 0x80000000 | 50,   49,   51,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 48, 0x80000000 | 50, QMetaType::Int,   49,   51,   56,
    QMetaType::Void, 0x80000000 | 48, 0x80000000 | 50, QMetaType::Int,   49,   51,   56,
    QMetaType::Void, QMetaType::QString,   59,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   59,   61,
    QMetaType::Void, 0x80000000 | 63,   64,
    QMetaType::Void, 0x80000000 | 66,   67,

       0        // eod
};

void TrayControlWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TrayControlWindow *_t = static_cast<TrayControlWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->show_about(); break;
        case 1: _t->show_settings_dialog(); break;
        case 2: _t->launch_Hub(); break;
        case 3: _t->launch_ss(); break;
        case 4: _t->show_create_dialog(); break;
        case 5: _t->show_notifications_triggered(); break;
        case 6: _t->launch_p2p(); break;
        case 7: _t->launch_p2p_installation(); break;
        case 8: _t->dialog_closed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->application_quit(); break;
        case 10: _t->notification_received((*reinterpret_cast< CNotificationObserver::notification_level_t(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< DlgNotification::NOTIFICATION_ACTION_TYPE(*)>(_a[3]))); break;
        case 11: _t->logout(); break;
        case 12: _t->login_success(); break;
        case 13: _t->environments_updated_sl((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->balance_updated_sl(); break;
        case 15: _t->my_peers_updated_sl(); break;
        case 16: _t->got_peer_info_sl((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 17: _t->machine_peers_upd_finished(); break;
        case 18: _t->peer_deleted_sl((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 19: _t->peer_under_modification_sl((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: _t->peer_poweroff_sl((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 21: _t->my_peer_button_pressed_sl((*reinterpret_cast< const my_peer_button*(*)>(_a[1]))); break;
        case 22: _t->got_ss_console_readiness_sl((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 23: _t->ssh_to_rh_triggered((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 24: _t->ssh_to_rh_finished_sl((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< system_call_wrapper_error_t(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 25: _t->upload_to_container_triggered((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2]))); break;
        case 26: _t->ssh_to_container_triggered((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2]))); break;
        case 27: _t->desktop_to_container_triggered((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2]))); break;
        case 28: _t->ssh_key_generate_triggered(); break;
        case 29: _t->ssh_to_container_finished((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 30: _t->desktop_to_container_finished((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 31: _t->update_available((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 32: _t->update_finished((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 33: _t->update_p2p_status_sl((*reinterpret_cast< P2PStatus_checker::P2P_STATUS(*)>(_a[1]))); break;
        case 34: _t->tray_icon_is_activated_sl((*reinterpret_cast< QSystemTrayIcon::ActivationReason(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TrayControlWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_TrayControlWindow.data,
      qt_meta_data_TrayControlWindow,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *TrayControlWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TrayControlWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TrayControlWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int TrayControlWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 35)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 35;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 35)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 35;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
