/****************************************************************************
** Meta object code from reading C++ file 'DlgPeer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/DlgPeer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgPeer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DlgPeer_t {
    QByteArrayData data[14];
    char stringdata0[193];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DlgPeer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DlgPeer_t qt_meta_stringdata_DlgPeer = {
    {
QT_MOC_LITERAL(0, 0, 7), // "DlgPeer"
QT_MOC_LITERAL(1, 8, 13), // "ssh_to_rh_sig"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 12), // "peer_deleted"
QT_MOC_LITERAL(4, 36, 13), // "peer_modified"
QT_MOC_LITERAL(5, 50, 12), // "peer_stopped"
QT_MOC_LITERAL(6, 63, 21), // "ssh_to_rh_finished_sl"
QT_MOC_LITERAL(7, 85, 16), // "peer_fingerprint"
QT_MOC_LITERAL(8, 102, 27), // "system_call_wrapper_error_t"
QT_MOC_LITERAL(9, 130, 3), // "res"
QT_MOC_LITERAL(10, 134, 17), // "libbssh_exit_code"
QT_MOC_LITERAL(11, 152, 12), // "registerPeer"
QT_MOC_LITERAL(12, 165, 14), // "unregisterPeer"
QT_MOC_LITERAL(13, 180, 12) // "regDlgClosed"

    },
    "DlgPeer\0ssh_to_rh_sig\0\0peer_deleted\0"
    "peer_modified\0peer_stopped\0"
    "ssh_to_rh_finished_sl\0peer_fingerprint\0"
    "system_call_wrapper_error_t\0res\0"
    "libbssh_exit_code\0registerPeer\0"
    "unregisterPeer\0regDlgClosed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DlgPeer[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       3,    1,   57,    2, 0x06 /* Public */,
       4,    1,   60,    2, 0x06 /* Public */,
       5,    1,   63,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    3,   66,    2, 0x08 /* Private */,
      11,    0,   73,    2, 0x08 /* Private */,
      12,    0,   74,    2, 0x08 /* Private */,
      13,    0,   75,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 8, QMetaType::Int,    7,    9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void DlgPeer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DlgPeer *_t = static_cast<DlgPeer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ssh_to_rh_sig((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->peer_deleted((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->peer_modified((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->peer_stopped((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->ssh_to_rh_finished_sl((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< system_call_wrapper_error_t(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 5: _t->registerPeer(); break;
        case 6: _t->unregisterPeer(); break;
        case 7: _t->regDlgClosed(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (DlgPeer::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgPeer::ssh_to_rh_sig)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DlgPeer::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgPeer::peer_deleted)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (DlgPeer::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgPeer::peer_modified)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (DlgPeer::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgPeer::peer_stopped)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DlgPeer::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgPeer.data,
      qt_meta_data_DlgPeer,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DlgPeer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DlgPeer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DlgPeer.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int DlgPeer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void DlgPeer::ssh_to_rh_sig(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DlgPeer::peer_deleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DlgPeer::peer_modified(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DlgPeer::peer_stopped(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
