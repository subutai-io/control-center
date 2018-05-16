/****************************************************************************
** Meta object code from reading C++ file 'DlgTransferFile.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/forms/DlgTransferFile.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgTransferFile.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FileThreadDownloader_t {
    QByteArrayData data[6];
    char stringdata0[76];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FileThreadDownloader_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FileThreadDownloader_t qt_meta_stringdata_FileThreadDownloader = {
    {
QT_MOC_LITERAL(0, 0, 20), // "FileThreadDownloader"
QT_MOC_LITERAL(1, 21, 14), // "outputReceived"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 27), // "system_call_wrapper_error_t"
QT_MOC_LITERAL(4, 65, 3), // "res"
QT_MOC_LITERAL(5, 69, 6) // "output"

    },
    "FileThreadDownloader\0outputReceived\0"
    "\0system_call_wrapper_error_t\0res\0"
    "output"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FileThreadDownloader[] = {

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
       1,    2,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    4,    5,

       0        // eod
};

void FileThreadDownloader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FileThreadDownloader *_t = static_cast<FileThreadDownloader *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->outputReceived((*reinterpret_cast< system_call_wrapper_error_t(*)>(_a[1])),(*reinterpret_cast< QStringList(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (FileThreadDownloader::*_t)(system_call_wrapper_error_t , QStringList );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileThreadDownloader::outputReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FileThreadDownloader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_FileThreadDownloader.data,
      qt_meta_data_FileThreadDownloader,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *FileThreadDownloader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FileThreadDownloader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FileThreadDownloader.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FileThreadDownloader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void FileThreadDownloader::outputReceived(system_call_wrapper_error_t _t1, QStringList _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_FileThreadUploader_t {
    QByteArrayData data[6];
    char stringdata0[74];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FileThreadUploader_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FileThreadUploader_t qt_meta_stringdata_FileThreadUploader = {
    {
QT_MOC_LITERAL(0, 0, 18), // "FileThreadUploader"
QT_MOC_LITERAL(1, 19, 14), // "outputReceived"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 27), // "system_call_wrapper_error_t"
QT_MOC_LITERAL(4, 63, 3), // "res"
QT_MOC_LITERAL(5, 67, 6) // "output"

    },
    "FileThreadUploader\0outputReceived\0\0"
    "system_call_wrapper_error_t\0res\0output"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FileThreadUploader[] = {

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
       1,    2,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    4,    5,

       0        // eod
};

void FileThreadUploader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FileThreadUploader *_t = static_cast<FileThreadUploader *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->outputReceived((*reinterpret_cast< system_call_wrapper_error_t(*)>(_a[1])),(*reinterpret_cast< QStringList(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (FileThreadUploader::*_t)(system_call_wrapper_error_t , QStringList );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileThreadUploader::outputReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FileThreadUploader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_FileThreadUploader.data,
      qt_meta_data_FileThreadUploader,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *FileThreadUploader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FileThreadUploader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FileThreadUploader.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FileThreadUploader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void FileThreadUploader::outputReceived(system_call_wrapper_error_t _t1, QStringList _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_RemoteCommandExecutor_t {
    QByteArrayData data[6];
    char stringdata0[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RemoteCommandExecutor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RemoteCommandExecutor_t qt_meta_stringdata_RemoteCommandExecutor = {
    {
QT_MOC_LITERAL(0, 0, 21), // "RemoteCommandExecutor"
QT_MOC_LITERAL(1, 22, 14), // "outputReceived"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 27), // "system_call_wrapper_error_t"
QT_MOC_LITERAL(4, 66, 3), // "res"
QT_MOC_LITERAL(5, 70, 6) // "output"

    },
    "RemoteCommandExecutor\0outputReceived\0"
    "\0system_call_wrapper_error_t\0res\0"
    "output"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RemoteCommandExecutor[] = {

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
       1,    2,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QStringList,    4,    5,

       0        // eod
};

void RemoteCommandExecutor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RemoteCommandExecutor *_t = static_cast<RemoteCommandExecutor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->outputReceived((*reinterpret_cast< system_call_wrapper_error_t(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (RemoteCommandExecutor::*_t)(system_call_wrapper_error_t , const QStringList & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RemoteCommandExecutor::outputReceived)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject RemoteCommandExecutor::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_RemoteCommandExecutor.data,
      qt_meta_data_RemoteCommandExecutor,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *RemoteCommandExecutor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RemoteCommandExecutor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RemoteCommandExecutor.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int RemoteCommandExecutor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void RemoteCommandExecutor::outputReceived(system_call_wrapper_error_t _t1, const QStringList & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_FileSystemTableWidget_t {
    QByteArrayData data[3];
    char stringdata0[44];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FileSystemTableWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FileSystemTableWidget_t qt_meta_stringdata_FileSystemTableWidget = {
    {
QT_MOC_LITERAL(0, 0, 21), // "FileSystemTableWidget"
QT_MOC_LITERAL(1, 22, 20), // "something_is_dropped"
QT_MOC_LITERAL(2, 43, 0) // ""

    },
    "FileSystemTableWidget\0something_is_dropped\0"
    ""
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FileSystemTableWidget[] = {

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
       1,    0,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,

       0        // eod
};

void FileSystemTableWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FileSystemTableWidget *_t = static_cast<FileSystemTableWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->something_is_dropped(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (FileSystemTableWidget::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileSystemTableWidget::something_is_dropped)) {
                *result = 0;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject FileSystemTableWidget::staticMetaObject = {
    { &QTableWidget::staticMetaObject, qt_meta_stringdata_FileSystemTableWidget.data,
      qt_meta_data_FileSystemTableWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *FileSystemTableWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FileSystemTableWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FileSystemTableWidget.stringdata0))
        return static_cast<void*>(this);
    return QTableWidget::qt_metacast(_clname);
}

int FileSystemTableWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableWidget::qt_metacall(_c, _id, _a);
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
void FileSystemTableWidget::something_is_dropped()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_DropFileTableWidget_t {
    QByteArrayData data[4];
    char stringdata0[48];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DropFileTableWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DropFileTableWidget_t qt_meta_stringdata_DropFileTableWidget = {
    {
QT_MOC_LITERAL(0, 0, 19), // "DropFileTableWidget"
QT_MOC_LITERAL(1, 20, 16), // "file_was_dropped"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 9) // "file_path"

    },
    "DropFileTableWidget\0file_was_dropped\0"
    "\0file_path"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DropFileTableWidget[] = {

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
    QMetaType::Void, QMetaType::QString,    3,

       0        // eod
};

void DropFileTableWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DropFileTableWidget *_t = static_cast<DropFileTableWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->file_was_dropped((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (DropFileTableWidget::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DropFileTableWidget::file_was_dropped)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DropFileTableWidget::staticMetaObject = {
    { &QTableWidget::staticMetaObject, qt_meta_stringdata_DropFileTableWidget.data,
      qt_meta_data_DropFileTableWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DropFileTableWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DropFileTableWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DropFileTableWidget.stringdata0))
        return static_cast<void*>(this);
    return QTableWidget::qt_metacast(_clname);
}

int DropFileTableWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableWidget::qt_metacall(_c, _id, _a);
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
void DropFileTableWidget::file_was_dropped(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_DlgTransferFile_t {
    QByteArrayData data[1];
    char stringdata0[16];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DlgTransferFile_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DlgTransferFile_t qt_meta_stringdata_DlgTransferFile = {
    {
QT_MOC_LITERAL(0, 0, 15) // "DlgTransferFile"

    },
    "DlgTransferFile"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DlgTransferFile[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void DlgTransferFile::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject DlgTransferFile::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgTransferFile.data,
      qt_meta_data_DlgTransferFile,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DlgTransferFile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DlgTransferFile::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DlgTransferFile.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int DlgTransferFile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
