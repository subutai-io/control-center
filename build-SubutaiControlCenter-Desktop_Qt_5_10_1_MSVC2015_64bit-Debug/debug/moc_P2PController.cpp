/****************************************************************************
** Meta object code from reading C++ file 'P2PController.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/P2PController.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'P2PController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_StatusChecker_t {
    QByteArrayData data[6];
    char stringdata0[79];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_StatusChecker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_StatusChecker_t qt_meta_stringdata_StatusChecker = {
    {
QT_MOC_LITERAL(0, 0, 13), // "StatusChecker"
QT_MOC_LITERAL(1, 14, 19), // "connection_finished"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 27), // "system_call_wrapper_error_t"
QT_MOC_LITERAL(4, 63, 3), // "res"
QT_MOC_LITERAL(5, 67, 11) // "run_checker"

    },
    "StatusChecker\0connection_finished\0\0"
    "system_call_wrapper_error_t\0res\0"
    "run_checker"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StatusChecker[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   27,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void StatusChecker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        StatusChecker *_t = static_cast<StatusChecker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connection_finished((*reinterpret_cast< system_call_wrapper_error_t(*)>(_a[1]))); break;
        case 1: _t->run_checker(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (StatusChecker::*_t)(system_call_wrapper_error_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StatusChecker::connection_finished)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject StatusChecker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_StatusChecker.data,
      qt_meta_data_StatusChecker,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *StatusChecker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StatusChecker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StatusChecker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int StatusChecker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void StatusChecker::connection_finished(system_call_wrapper_error_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_RHStatusChecker_t {
    QByteArrayData data[3];
    char stringdata0[29];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RHStatusChecker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RHStatusChecker_t qt_meta_stringdata_RHStatusChecker = {
    {
QT_MOC_LITERAL(0, 0, 15), // "RHStatusChecker"
QT_MOC_LITERAL(1, 16, 11), // "run_checker"
QT_MOC_LITERAL(2, 28, 0) // ""

    },
    "RHStatusChecker\0run_checker\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RHStatusChecker[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void RHStatusChecker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RHStatusChecker *_t = static_cast<RHStatusChecker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->run_checker(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject RHStatusChecker::staticMetaObject = {
    { &StatusChecker::staticMetaObject, qt_meta_stringdata_RHStatusChecker.data,
      qt_meta_data_RHStatusChecker,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *RHStatusChecker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RHStatusChecker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RHStatusChecker.stringdata0))
        return static_cast<void*>(this);
    return StatusChecker::qt_metacast(_clname);
}

int RHStatusChecker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = StatusChecker::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_SwarmConnector_t {
    QByteArrayData data[3];
    char stringdata0[28];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SwarmConnector_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SwarmConnector_t qt_meta_stringdata_SwarmConnector = {
    {
QT_MOC_LITERAL(0, 0, 14), // "SwarmConnector"
QT_MOC_LITERAL(1, 15, 11), // "run_checker"
QT_MOC_LITERAL(2, 27, 0) // ""

    },
    "SwarmConnector\0run_checker\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SwarmConnector[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void SwarmConnector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SwarmConnector *_t = static_cast<SwarmConnector *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->run_checker(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject SwarmConnector::staticMetaObject = {
    { &StatusChecker::staticMetaObject, qt_meta_stringdata_SwarmConnector.data,
      qt_meta_data_SwarmConnector,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SwarmConnector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SwarmConnector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SwarmConnector.stringdata0))
        return static_cast<void*>(this);
    return StatusChecker::qt_metacast(_clname);
}

int SwarmConnector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = StatusChecker::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_SwarmLeaver_t {
    QByteArrayData data[3];
    char stringdata0[25];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SwarmLeaver_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SwarmLeaver_t qt_meta_stringdata_SwarmLeaver = {
    {
QT_MOC_LITERAL(0, 0, 11), // "SwarmLeaver"
QT_MOC_LITERAL(1, 12, 11), // "run_checker"
QT_MOC_LITERAL(2, 24, 0) // ""

    },
    "SwarmLeaver\0run_checker\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SwarmLeaver[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void SwarmLeaver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SwarmLeaver *_t = static_cast<SwarmLeaver *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->run_checker(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject SwarmLeaver::staticMetaObject = {
    { &StatusChecker::staticMetaObject, qt_meta_stringdata_SwarmLeaver.data,
      qt_meta_data_SwarmLeaver,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SwarmLeaver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SwarmLeaver::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SwarmLeaver.stringdata0))
        return static_cast<void*>(this);
    return StatusChecker::qt_metacast(_clname);
}

int SwarmLeaver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = StatusChecker::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_P2PConnector_t {
    QByteArrayData data[3];
    char stringdata0[28];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_P2PConnector_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_P2PConnector_t qt_meta_stringdata_P2PConnector = {
    {
QT_MOC_LITERAL(0, 0, 12), // "P2PConnector"
QT_MOC_LITERAL(1, 13, 13), // "update_status"
QT_MOC_LITERAL(2, 27, 0) // ""

    },
    "P2PConnector\0update_status\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_P2PConnector[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void P2PConnector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        P2PConnector *_t = static_cast<P2PConnector *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->update_status(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject P2PConnector::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_P2PConnector.data,
      qt_meta_data_P2PConnector,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *P2PConnector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *P2PConnector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_P2PConnector.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int P2PConnector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_P2PController_t {
    QByteArrayData data[9];
    char stringdata0[132];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_P2PController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_P2PController_t qt_meta_stringdata_P2PController = {
    {
QT_MOC_LITERAL(0, 0, 13), // "P2PController"
QT_MOC_LITERAL(1, 14, 21), // "p2p_status_updated_sl"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 25), // "std::vector<CP2PInstance>"
QT_MOC_LITERAL(4, 63, 17), // "new_p2p_instances"
QT_MOC_LITERAL(5, 81, 9), // "http_code"
QT_MOC_LITERAL(6, 91, 8), // "err_code"
QT_MOC_LITERAL(7, 100, 13), // "network_error"
QT_MOC_LITERAL(8, 114, 17) // "update_p2p_status"

    },
    "P2PController\0p2p_status_updated_sl\0"
    "\0std::vector<CP2PInstance>\0new_p2p_instances\0"
    "http_code\0err_code\0network_error\0"
    "update_p2p_status"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_P2PController[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    4,   24,    2, 0x08 /* Private */,
       8,    0,   33,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Int, QMetaType::Int,    4,    5,    6,    7,
    QMetaType::Void,

       0        // eod
};

void P2PController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        P2PController *_t = static_cast<P2PController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->p2p_status_updated_sl((*reinterpret_cast< std::vector<CP2PInstance>(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 1: _t->update_p2p_status(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject P2PController::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_P2PController.data,
      qt_meta_data_P2PController,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *P2PController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *P2PController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_P2PController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int P2PController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
struct qt_meta_stringdata_P2PStatus_checker_t {
    QByteArrayData data[8];
    char stringdata0[90];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_P2PStatus_checker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_P2PStatus_checker_t qt_meta_stringdata_P2PStatus_checker = {
    {
QT_MOC_LITERAL(0, 0, 17), // "P2PStatus_checker"
QT_MOC_LITERAL(1, 18, 10), // "p2p_status"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 10), // "P2P_STATUS"
QT_MOC_LITERAL(4, 41, 15), // "install_started"
QT_MOC_LITERAL(5, 57, 7), // "file_id"
QT_MOC_LITERAL(6, 65, 16), // "install_finished"
QT_MOC_LITERAL(7, 82, 7) // "success"

    },
    "P2PStatus_checker\0p2p_status\0\0P2P_STATUS\0"
    "install_started\0file_id\0install_finished\0"
    "success"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_P2PStatus_checker[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   32,    2, 0x08 /* Private */,
       6,    2,   35,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    5,    7,

       0        // eod
};

void P2PStatus_checker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        P2PStatus_checker *_t = static_cast<P2PStatus_checker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->p2p_status((*reinterpret_cast< P2P_STATUS(*)>(_a[1]))); break;
        case 1: _t->install_started((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->install_finished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (P2PStatus_checker::*_t)(P2P_STATUS );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&P2PStatus_checker::p2p_status)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject P2PStatus_checker::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_P2PStatus_checker.data,
      qt_meta_data_P2PStatus_checker,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *P2PStatus_checker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *P2PStatus_checker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_P2PStatus_checker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int P2PStatus_checker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void P2PStatus_checker::p2p_status(P2P_STATUS _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
