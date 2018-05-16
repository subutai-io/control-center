/****************************************************************************
** Meta object code from reading C++ file 'DownloadFileManager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/DownloadFileManager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DownloadFileManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CDownloadFileManager_t {
    QByteArrayData data[14];
    char stringdata0[186];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDownloadFileManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDownloadFileManager_t qt_meta_stringdata_CDownloadFileManager = {
    {
QT_MOC_LITERAL(0, 0, 20), // "CDownloadFileManager"
QT_MOC_LITERAL(1, 21, 8), // "finished"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 7), // "success"
QT_MOC_LITERAL(4, 39, 21), // "download_progress_sig"
QT_MOC_LITERAL(5, 61, 3), // "rec"
QT_MOC_LITERAL(6, 65, 5), // "total"
QT_MOC_LITERAL(7, 71, 17), // "download_progress"
QT_MOC_LITERAL(8, 89, 10), // "read_bytes"
QT_MOC_LITERAL(9, 100, 11), // "total_bytes"
QT_MOC_LITERAL(10, 112, 24), // "network_reply_ready_read"
QT_MOC_LITERAL(11, 137, 14), // "reply_finished"
QT_MOC_LITERAL(12, 152, 14), // "start_download"
QT_MOC_LITERAL(13, 167, 18) // "interrupt_download"

    },
    "CDownloadFileManager\0finished\0\0success\0"
    "download_progress_sig\0rec\0total\0"
    "download_progress\0read_bytes\0total_bytes\0"
    "network_reply_ready_read\0reply_finished\0"
    "start_download\0interrupt_download"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDownloadFileManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       4,    2,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    2,   57,    2, 0x08 /* Private */,
      10,    0,   62,    2, 0x08 /* Private */,
      11,    0,   63,    2, 0x08 /* Private */,
      12,    0,   64,    2, 0x0a /* Public */,
      13,    0,   65,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::LongLong, QMetaType::LongLong,    5,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::LongLong, QMetaType::LongLong,    8,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CDownloadFileManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CDownloadFileManager *_t = static_cast<CDownloadFileManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->finished((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->download_progress_sig((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 2: _t->download_progress((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 3: _t->network_reply_ready_read(); break;
        case 4: _t->reply_finished(); break;
        case 5: _t->start_download(); break;
        case 6: _t->interrupt_download(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (CDownloadFileManager::*_t)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CDownloadFileManager::finished)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (CDownloadFileManager::*_t)(qint64 , qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CDownloadFileManager::download_progress_sig)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CDownloadFileManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CDownloadFileManager.data,
      qt_meta_data_CDownloadFileManager,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CDownloadFileManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDownloadFileManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDownloadFileManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CDownloadFileManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void CDownloadFileManager::finished(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CDownloadFileManager::download_progress_sig(qint64 _t1, qint64 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
