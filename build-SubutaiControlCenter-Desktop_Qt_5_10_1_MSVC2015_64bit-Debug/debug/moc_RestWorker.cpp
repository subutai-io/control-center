/****************************************************************************
** Meta object code from reading C++ file 'RestWorker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/RestWorker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RestWorker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CRestWorker_t {
    QByteArrayData data[21];
    char stringdata0[427];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CRestWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CRestWorker_t qt_meta_stringdata_CRestWorker = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CRestWorker"
QT_MOC_LITERAL(1, 12, 24), // "on_get_my_peers_finished"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 24), // "std::vector<CMyPeerInfo>"
QT_MOC_LITERAL(4, 63, 9), // "http_code"
QT_MOC_LITERAL(5, 73, 8), // "err_code"
QT_MOC_LITERAL(6, 82, 13), // "network_error"
QT_MOC_LITERAL(7, 96, 26), // "on_get_p2p_status_finished"
QT_MOC_LITERAL(8, 123, 25), // "std::vector<CP2PInstance>"
QT_MOC_LITERAL(9, 149, 28), // "on_get_environments_finished"
QT_MOC_LITERAL(10, 178, 25), // "std::vector<CEnvironment>"
QT_MOC_LITERAL(11, 204, 23), // "on_get_balance_finished"
QT_MOC_LITERAL(12, 228, 11), // "CHubBalance"
QT_MOC_LITERAL(13, 240, 27), // "on_got_ss_console_readiness"
QT_MOC_LITERAL(14, 268, 8), // "is_ready"
QT_MOC_LITERAL(15, 277, 3), // "err"
QT_MOC_LITERAL(16, 281, 24), // "get_my_peers_finished_sl"
QT_MOC_LITERAL(17, 306, 28), // "get_environments_finished_sl"
QT_MOC_LITERAL(18, 335, 23), // "get_balance_finished_sl"
QT_MOC_LITERAL(19, 359, 40), // "check_if_ss_console_is_ready_..."
QT_MOC_LITERAL(20, 400, 26) // "get_p2p_status_finished_sl"

    },
    "CRestWorker\0on_get_my_peers_finished\0"
    "\0std::vector<CMyPeerInfo>\0http_code\0"
    "err_code\0network_error\0"
    "on_get_p2p_status_finished\0"
    "std::vector<CP2PInstance>\0"
    "on_get_environments_finished\0"
    "std::vector<CEnvironment>\0"
    "on_get_balance_finished\0CHubBalance\0"
    "on_got_ss_console_readiness\0is_ready\0"
    "err\0get_my_peers_finished_sl\0"
    "get_environments_finished_sl\0"
    "get_balance_finished_sl\0"
    "check_if_ss_console_is_ready_finished_sl\0"
    "get_p2p_status_finished_sl"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CRestWorker[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    4,   64,    2, 0x06 /* Public */,
       7,    4,   73,    2, 0x06 /* Public */,
       9,    4,   82,    2, 0x06 /* Public */,
      11,    4,   91,    2, 0x06 /* Public */,
      13,    2,  100,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    0,  105,    2, 0x08 /* Private */,
      17,    0,  106,    2, 0x08 /* Private */,
      18,    0,  107,    2, 0x08 /* Private */,
      19,    0,  108,    2, 0x08 /* Private */,
      20,    0,  109,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int, QMetaType::Int,    2,    4,    5,    6,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int, QMetaType::Int, QMetaType::Int,    2,    4,    5,    6,
    QMetaType::Void, 0x80000000 | 10, QMetaType::Int, QMetaType::Int, QMetaType::Int,    2,    4,    5,    6,
    QMetaType::Void, 0x80000000 | 12, QMetaType::Int, QMetaType::Int, QMetaType::Int,    2,    4,    5,    6,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   14,   15,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CRestWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CRestWorker *_t = static_cast<CRestWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_get_my_peers_finished((*reinterpret_cast< std::vector<CMyPeerInfo>(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 1: _t->on_get_p2p_status_finished((*reinterpret_cast< std::vector<CP2PInstance>(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 2: _t->on_get_environments_finished((*reinterpret_cast< std::vector<CEnvironment>(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 3: _t->on_get_balance_finished((*reinterpret_cast< CHubBalance(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 4: _t->on_got_ss_console_readiness((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->get_my_peers_finished_sl(); break;
        case 6: _t->get_environments_finished_sl(); break;
        case 7: _t->get_balance_finished_sl(); break;
        case 8: _t->check_if_ss_console_is_ready_finished_sl(); break;
        case 9: _t->get_p2p_status_finished_sl(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (CRestWorker::*_t)(std::vector<CMyPeerInfo> , int , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CRestWorker::on_get_my_peers_finished)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (CRestWorker::*_t)(std::vector<CP2PInstance> , int , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CRestWorker::on_get_p2p_status_finished)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (CRestWorker::*_t)(std::vector<CEnvironment> , int , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CRestWorker::on_get_environments_finished)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (CRestWorker::*_t)(CHubBalance , int , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CRestWorker::on_get_balance_finished)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (CRestWorker::*_t)(bool , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CRestWorker::on_got_ss_console_readiness)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CRestWorker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CRestWorker.data,
      qt_meta_data_CRestWorker,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CRestWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CRestWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CRestWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CRestWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void CRestWorker::on_get_my_peers_finished(std::vector<CMyPeerInfo> _t1, int _t2, int _t3, int _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CRestWorker::on_get_p2p_status_finished(std::vector<CP2PInstance> _t1, int _t2, int _t3, int _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CRestWorker::on_get_environments_finished(std::vector<CEnvironment> _t1, int _t2, int _t3, int _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CRestWorker::on_get_balance_finished(CHubBalance _t1, int _t2, int _t3, int _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void CRestWorker::on_got_ss_console_readiness(bool _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
