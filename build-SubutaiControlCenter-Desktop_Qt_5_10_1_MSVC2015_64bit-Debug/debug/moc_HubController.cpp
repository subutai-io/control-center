/****************************************************************************
** Meta object code from reading C++ file 'HubController.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/HubController.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HubController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CHubController_t {
    QByteArrayData data[30];
    char stringdata0[542];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CHubController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CHubController_t qt_meta_stringdata_CHubController = {
    {
QT_MOC_LITERAL(0, 0, 14), // "CHubController"
QT_MOC_LITERAL(1, 15, 34), // "ssh_to_container_from_hub_fin..."
QT_MOC_LITERAL(2, 50, 0), // ""
QT_MOC_LITERAL(3, 51, 12), // "CEnvironment"
QT_MOC_LITERAL(4, 64, 3), // "env"
QT_MOC_LITERAL(5, 68, 13), // "CHubContainer"
QT_MOC_LITERAL(6, 82, 4), // "cont"
QT_MOC_LITERAL(7, 87, 6), // "result"
QT_MOC_LITERAL(8, 94, 15), // "additional_data"
QT_MOC_LITERAL(9, 110, 38), // "desktop_to_container_from_hub..."
QT_MOC_LITERAL(10, 149, 35), // "ssh_to_container_from_tray_fi..."
QT_MOC_LITERAL(11, 185, 39), // "desktop_to_container_from_tra..."
QT_MOC_LITERAL(12, 225, 20), // "environments_updated"
QT_MOC_LITERAL(13, 246, 16), // "my_peers_updated"
QT_MOC_LITERAL(14, 263, 15), // "balance_updated"
QT_MOC_LITERAL(15, 279, 16), // "settings_changed"
QT_MOC_LITERAL(16, 296, 20), // "report_timer_timeout"
QT_MOC_LITERAL(17, 317, 21), // "refresh_timer_timeout"
QT_MOC_LITERAL(18, 339, 21), // "on_balance_updated_sl"
QT_MOC_LITERAL(19, 361, 11), // "CHubBalance"
QT_MOC_LITERAL(20, 373, 7), // "balance"
QT_MOC_LITERAL(21, 381, 9), // "http_code"
QT_MOC_LITERAL(22, 391, 8), // "err_code"
QT_MOC_LITERAL(23, 400, 13), // "network_error"
QT_MOC_LITERAL(24, 414, 26), // "on_environments_updated_sl"
QT_MOC_LITERAL(25, 441, 25), // "std::vector<CEnvironment>"
QT_MOC_LITERAL(26, 467, 16), // "lst_environments"
QT_MOC_LITERAL(27, 484, 22), // "on_my_peers_updated_sl"
QT_MOC_LITERAL(28, 507, 24), // "std::vector<CMyPeerInfo>"
QT_MOC_LITERAL(29, 532, 9) // "lst_peers"

    },
    "CHubController\0ssh_to_container_from_hub_finished\0"
    "\0CEnvironment\0env\0CHubContainer\0cont\0"
    "result\0additional_data\0"
    "desktop_to_container_from_hub_finished\0"
    "ssh_to_container_from_tray_finished\0"
    "desktop_to_container_from_tray_finished\0"
    "environments_updated\0my_peers_updated\0"
    "balance_updated\0settings_changed\0"
    "report_timer_timeout\0refresh_timer_timeout\0"
    "on_balance_updated_sl\0CHubBalance\0"
    "balance\0http_code\0err_code\0network_error\0"
    "on_environments_updated_sl\0"
    "std::vector<CEnvironment>\0lst_environments\0"
    "on_my_peers_updated_sl\0std::vector<CMyPeerInfo>\0"
    "lst_peers"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CHubController[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    4,   79,    2, 0x06 /* Public */,
       9,    4,   88,    2, 0x06 /* Public */,
      10,    3,   97,    2, 0x06 /* Public */,
      11,    3,  104,    2, 0x06 /* Public */,
      12,    1,  111,    2, 0x06 /* Public */,
      13,    0,  114,    2, 0x06 /* Public */,
      14,    0,  115,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      15,    0,  116,    2, 0x08 /* Private */,
      16,    0,  117,    2, 0x08 /* Private */,
      17,    0,  118,    2, 0x08 /* Private */,
      18,    4,  119,    2, 0x08 /* Private */,
      24,    4,  128,    2, 0x08 /* Private */,
      27,    4,  137,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, QMetaType::Int, QMetaType::VoidStar,    4,    6,    7,    8,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, QMetaType::Int, QMetaType::VoidStar,    4,    6,    7,    8,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, QMetaType::Int,    4,    6,    7,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, QMetaType::Int,    4,    6,    7,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 19, QMetaType::Int, QMetaType::Int, QMetaType::Int,   20,   21,   22,   23,
    QMetaType::Void, 0x80000000 | 25, QMetaType::Int, QMetaType::Int, QMetaType::Int,   26,   21,   22,   23,
    QMetaType::Void, 0x80000000 | 28, QMetaType::Int, QMetaType::Int, QMetaType::Int,   29,   21,   22,   23,

       0        // eod
};

void CHubController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CHubController *_t = static_cast<CHubController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ssh_to_container_from_hub_finished((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4]))); break;
        case 1: _t->desktop_to_container_from_hub_finished((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4]))); break;
        case 2: _t->ssh_to_container_from_tray_finished((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: _t->desktop_to_container_from_tray_finished((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 4: _t->environments_updated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->my_peers_updated(); break;
        case 6: _t->balance_updated(); break;
        case 7: _t->settings_changed(); break;
        case 8: _t->report_timer_timeout(); break;
        case 9: _t->refresh_timer_timeout(); break;
        case 10: _t->on_balance_updated_sl((*reinterpret_cast< CHubBalance(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 11: _t->on_environments_updated_sl((*reinterpret_cast< std::vector<CEnvironment>(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 12: _t->on_my_peers_updated_sl((*reinterpret_cast< std::vector<CMyPeerInfo>(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (CHubController::*_t)(const CEnvironment & , const CHubContainer & , int , void * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubController::ssh_to_container_from_hub_finished)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (CHubController::*_t)(const CEnvironment & , const CHubContainer & , int , void * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubController::desktop_to_container_from_hub_finished)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (CHubController::*_t)(const CEnvironment & , const CHubContainer & , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubController::ssh_to_container_from_tray_finished)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (CHubController::*_t)(const CEnvironment & , const CHubContainer & , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubController::desktop_to_container_from_tray_finished)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (CHubController::*_t)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubController::environments_updated)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (CHubController::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubController::my_peers_updated)) {
                *result = 5;
                return;
            }
        }
        {
            typedef void (CHubController::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubController::balance_updated)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CHubController::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CHubController.data,
      qt_meta_data_CHubController,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CHubController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CHubController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CHubController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CHubController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void CHubController::ssh_to_container_from_hub_finished(const CEnvironment & _t1, const CHubContainer & _t2, int _t3, void * _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CHubController::desktop_to_container_from_hub_finished(const CEnvironment & _t1, const CHubContainer & _t2, int _t3, void * _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CHubController::ssh_to_container_from_tray_finished(const CEnvironment & _t1, const CHubContainer & _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CHubController::desktop_to_container_from_tray_finished(const CEnvironment & _t1, const CHubContainer & _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void CHubController::environments_updated(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void CHubController::my_peers_updated()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void CHubController::balance_updated()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
