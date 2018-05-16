/****************************************************************************
** Meta object code from reading C++ file 'HubComponentsUpdater.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/updater/HubComponentsUpdater.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HubComponentsUpdater.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_update_system__CUpdaterComponentItem_t {
    QByteArrayData data[5];
    char stringdata0[82];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_update_system__CUpdaterComponentItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_update_system__CUpdaterComponentItem_t qt_meta_stringdata_update_system__CUpdaterComponentItem = {
    {
QT_MOC_LITERAL(0, 0, 36), // "update_system::CUpdaterCompon..."
QT_MOC_LITERAL(1, 37, 13), // "timer_timeout"
QT_MOC_LITERAL(2, 51, 0), // ""
QT_MOC_LITERAL(3, 52, 12), // "component_id"
QT_MOC_LITERAL(4, 65, 16) // "timer_timeout_sl"

    },
    "update_system::CUpdaterComponentItem\0"
    "timer_timeout\0\0component_id\0"
    "timer_timeout_sl"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_update_system__CUpdaterComponentItem[] = {

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
       4,    0,   27,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void update_system::CUpdaterComponentItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CUpdaterComponentItem *_t = static_cast<CUpdaterComponentItem *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->timer_timeout((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->timer_timeout_sl(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (CUpdaterComponentItem::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CUpdaterComponentItem::timer_timeout)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject update_system::CUpdaterComponentItem::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_update_system__CUpdaterComponentItem.data,
      qt_meta_data_update_system__CUpdaterComponentItem,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *update_system::CUpdaterComponentItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *update_system::CUpdaterComponentItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_update_system__CUpdaterComponentItem.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int update_system::CUpdaterComponentItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void update_system::CUpdaterComponentItem::timer_timeout(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_update_system__CHubComponentsUpdater_t {
    QByteArrayData data[19];
    char stringdata0[318];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_update_system__CHubComponentsUpdater_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_update_system__CHubComponentsUpdater_t qt_meta_stringdata_update_system__CHubComponentsUpdater = {
    {
QT_MOC_LITERAL(0, 0, 36), // "update_system::CHubComponents..."
QT_MOC_LITERAL(1, 37, 22), // "download_file_progress"
QT_MOC_LITERAL(2, 60, 0), // ""
QT_MOC_LITERAL(3, 61, 7), // "file_id"
QT_MOC_LITERAL(4, 69, 3), // "rec"
QT_MOC_LITERAL(5, 73, 5), // "total"
QT_MOC_LITERAL(6, 79, 17), // "updating_finished"
QT_MOC_LITERAL(7, 97, 7), // "success"
QT_MOC_LITERAL(8, 105, 16), // "update_available"
QT_MOC_LITERAL(9, 122, 19), // "installing_finished"
QT_MOC_LITERAL(10, 142, 25), // "install_component_started"
QT_MOC_LITERAL(11, 168, 30), // "update_component_timer_timeout"
QT_MOC_LITERAL(12, 199, 12), // "component_id"
QT_MOC_LITERAL(13, 212, 28), // "update_component_progress_sl"
QT_MOC_LITERAL(14, 241, 3), // "cur"
QT_MOC_LITERAL(15, 245, 4), // "full"
QT_MOC_LITERAL(16, 250, 28), // "update_component_finished_sl"
QT_MOC_LITERAL(17, 279, 8), // "replaced"
QT_MOC_LITERAL(18, 288, 29) // "install_component_finished_sl"

    },
    "update_system::CHubComponentsUpdater\0"
    "download_file_progress\0\0file_id\0rec\0"
    "total\0updating_finished\0success\0"
    "update_available\0installing_finished\0"
    "install_component_started\0"
    "update_component_timer_timeout\0"
    "component_id\0update_component_progress_sl\0"
    "cur\0full\0update_component_finished_sl\0"
    "replaced\0install_component_finished_sl"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_update_system__CHubComponentsUpdater[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   59,    2, 0x06 /* Public */,
       6,    2,   66,    2, 0x06 /* Public */,
       8,    1,   71,    2, 0x06 /* Public */,
       9,    2,   74,    2, 0x06 /* Public */,
      10,    1,   79,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    1,   82,    2, 0x08 /* Private */,
      13,    3,   85,    2, 0x08 /* Private */,
      16,    2,   92,    2, 0x08 /* Private */,
      18,    2,   97,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong, QMetaType::LongLong,    3,    4,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    7,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    7,
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong, QMetaType::LongLong,    3,   14,   15,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,   17,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,   17,

       0        // eod
};

void update_system::CHubComponentsUpdater::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CHubComponentsUpdater *_t = static_cast<CHubComponentsUpdater *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->download_file_progress((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2])),(*reinterpret_cast< qint64(*)>(_a[3]))); break;
        case 1: _t->updating_finished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->update_available((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->installing_finished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->install_component_started((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->update_component_timer_timeout((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->update_component_progress_sl((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2])),(*reinterpret_cast< qint64(*)>(_a[3]))); break;
        case 7: _t->update_component_finished_sl((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 8: _t->install_component_finished_sl((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (CHubComponentsUpdater::*_t)(const QString & , qint64 , qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubComponentsUpdater::download_file_progress)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (CHubComponentsUpdater::*_t)(const QString & , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubComponentsUpdater::updating_finished)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (CHubComponentsUpdater::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubComponentsUpdater::update_available)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (CHubComponentsUpdater::*_t)(const QString & , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubComponentsUpdater::installing_finished)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (CHubComponentsUpdater::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CHubComponentsUpdater::install_component_started)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject update_system::CHubComponentsUpdater::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_update_system__CHubComponentsUpdater.data,
      qt_meta_data_update_system__CHubComponentsUpdater,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *update_system::CHubComponentsUpdater::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *update_system::CHubComponentsUpdater::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_update_system__CHubComponentsUpdater.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int update_system::CHubComponentsUpdater::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void update_system::CHubComponentsUpdater::download_file_progress(const QString & _t1, qint64 _t2, qint64 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void update_system::CHubComponentsUpdater::updating_finished(const QString & _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void update_system::CHubComponentsUpdater::update_available(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void update_system::CHubComponentsUpdater::installing_finished(const QString & _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void update_system::CHubComponentsUpdater::install_component_started(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
struct qt_meta_stringdata_SilentPackageInstallerP2P_t {
    QByteArrayData data[4];
    char stringdata0[50];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SilentPackageInstallerP2P_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SilentPackageInstallerP2P_t qt_meta_stringdata_SilentPackageInstallerP2P = {
    {
QT_MOC_LITERAL(0, 0, 25), // "SilentPackageInstallerP2P"
QT_MOC_LITERAL(1, 26, 14), // "outputReceived"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 7) // "success"

    },
    "SilentPackageInstallerP2P\0outputReceived\0"
    "\0success"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SilentPackageInstallerP2P[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,

       0        // eod
};

void SilentPackageInstallerP2P::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SilentPackageInstallerP2P *_t = static_cast<SilentPackageInstallerP2P *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->outputReceived((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (SilentPackageInstallerP2P::*_t)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SilentPackageInstallerP2P::outputReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SilentPackageInstallerP2P::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SilentPackageInstallerP2P.data,
      qt_meta_data_SilentPackageInstallerP2P,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SilentPackageInstallerP2P::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SilentPackageInstallerP2P::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SilentPackageInstallerP2P.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SilentPackageInstallerP2P::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void SilentPackageInstallerP2P::outputReceived(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_SilentPackageInstallerX2GO_t {
    QByteArrayData data[4];
    char stringdata0[51];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SilentPackageInstallerX2GO_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SilentPackageInstallerX2GO_t qt_meta_stringdata_SilentPackageInstallerX2GO = {
    {
QT_MOC_LITERAL(0, 0, 26), // "SilentPackageInstallerX2GO"
QT_MOC_LITERAL(1, 27, 14), // "outputReceived"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 7) // "success"

    },
    "SilentPackageInstallerX2GO\0outputReceived\0"
    "\0success"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SilentPackageInstallerX2GO[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,

       0        // eod
};

void SilentPackageInstallerX2GO::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SilentPackageInstallerX2GO *_t = static_cast<SilentPackageInstallerX2GO *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->outputReceived((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (SilentPackageInstallerX2GO::*_t)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SilentPackageInstallerX2GO::outputReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SilentPackageInstallerX2GO::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SilentPackageInstallerX2GO.data,
      qt_meta_data_SilentPackageInstallerX2GO,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SilentPackageInstallerX2GO::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SilentPackageInstallerX2GO::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SilentPackageInstallerX2GO.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SilentPackageInstallerX2GO::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void SilentPackageInstallerX2GO::outputReceived(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_SilentPackageInstallerVAGRANT_t {
    QByteArrayData data[4];
    char stringdata0[54];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SilentPackageInstallerVAGRANT_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SilentPackageInstallerVAGRANT_t qt_meta_stringdata_SilentPackageInstallerVAGRANT = {
    {
QT_MOC_LITERAL(0, 0, 29), // "SilentPackageInstallerVAGRANT"
QT_MOC_LITERAL(1, 30, 14), // "outputReceived"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 7) // "success"

    },
    "SilentPackageInstallerVAGRANT\0"
    "outputReceived\0\0success"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SilentPackageInstallerVAGRANT[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,

       0        // eod
};

void SilentPackageInstallerVAGRANT::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SilentPackageInstallerVAGRANT *_t = static_cast<SilentPackageInstallerVAGRANT *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->outputReceived((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (SilentPackageInstallerVAGRANT::*_t)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SilentPackageInstallerVAGRANT::outputReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SilentPackageInstallerVAGRANT::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SilentPackageInstallerVAGRANT.data,
      qt_meta_data_SilentPackageInstallerVAGRANT,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SilentPackageInstallerVAGRANT::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SilentPackageInstallerVAGRANT::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SilentPackageInstallerVAGRANT.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SilentPackageInstallerVAGRANT::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void SilentPackageInstallerVAGRANT::outputReceived(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_SilentPackageInstallerORACLE_VIRTUALBOX_t {
    QByteArrayData data[4];
    char stringdata0[64];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SilentPackageInstallerORACLE_VIRTUALBOX_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SilentPackageInstallerORACLE_VIRTUALBOX_t qt_meta_stringdata_SilentPackageInstallerORACLE_VIRTUALBOX = {
    {
QT_MOC_LITERAL(0, 0, 39), // "SilentPackageInstallerORACLE_..."
QT_MOC_LITERAL(1, 40, 14), // "outputReceived"
QT_MOC_LITERAL(2, 55, 0), // ""
QT_MOC_LITERAL(3, 56, 7) // "success"

    },
    "SilentPackageInstallerORACLE_VIRTUALBOX\0"
    "outputReceived\0\0success"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SilentPackageInstallerORACLE_VIRTUALBOX[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,

       0        // eod
};

void SilentPackageInstallerORACLE_VIRTUALBOX::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SilentPackageInstallerORACLE_VIRTUALBOX *_t = static_cast<SilentPackageInstallerORACLE_VIRTUALBOX *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->outputReceived((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (SilentPackageInstallerORACLE_VIRTUALBOX::*_t)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SilentPackageInstallerORACLE_VIRTUALBOX::outputReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SilentPackageInstallerORACLE_VIRTUALBOX::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SilentPackageInstallerORACLE_VIRTUALBOX.data,
      qt_meta_data_SilentPackageInstallerORACLE_VIRTUALBOX,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SilentPackageInstallerORACLE_VIRTUALBOX::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SilentPackageInstallerORACLE_VIRTUALBOX::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SilentPackageInstallerORACLE_VIRTUALBOX.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SilentPackageInstallerORACLE_VIRTUALBOX::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void SilentPackageInstallerORACLE_VIRTUALBOX::outputReceived(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
