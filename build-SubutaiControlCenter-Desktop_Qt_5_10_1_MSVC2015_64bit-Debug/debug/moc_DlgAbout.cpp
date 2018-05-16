/****************************************************************************
** Meta object code from reading C++ file 'DlgAbout.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/DlgAbout.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgAbout.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DlgAboutInitializer_t {
    QByteArrayData data[17];
    char stringdata0[246];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DlgAboutInitializer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DlgAboutInitializer_t qt_meta_stringdata_DlgAboutInitializer = {
    {
QT_MOC_LITERAL(0, 0, 19), // "DlgAboutInitializer"
QT_MOC_LITERAL(1, 20, 13), // "init_progress"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 4), // "part"
QT_MOC_LITERAL(4, 40, 5), // "total"
QT_MOC_LITERAL(5, 46, 8), // "finished"
QT_MOC_LITERAL(6, 55, 15), // "got_p2p_version"
QT_MOC_LITERAL(7, 71, 7), // "version"
QT_MOC_LITERAL(8, 79, 18), // "got_chrome_version"
QT_MOC_LITERAL(9, 98, 14), // "got_rh_version"
QT_MOC_LITERAL(10, 113, 25), // "got_rh_management_version"
QT_MOC_LITERAL(11, 139, 16), // "got_x2go_version"
QT_MOC_LITERAL(12, 156, 19), // "got_vagrant_version"
QT_MOC_LITERAL(13, 176, 29), // "got_oracle_virtualbox_version"
QT_MOC_LITERAL(14, 206, 16), // "update_available"
QT_MOC_LITERAL(15, 223, 12), // "component_id"
QT_MOC_LITERAL(16, 236, 9) // "available"

    },
    "DlgAboutInitializer\0init_progress\0\0"
    "part\0total\0finished\0got_p2p_version\0"
    "version\0got_chrome_version\0got_rh_version\0"
    "got_rh_management_version\0got_x2go_version\0"
    "got_vagrant_version\0got_oracle_virtualbox_version\0"
    "update_available\0component_id\0available"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DlgAboutInitializer[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x06 /* Public */,
       5,    0,   69,    2, 0x06 /* Public */,
       6,    1,   70,    2, 0x06 /* Public */,
       8,    1,   73,    2, 0x06 /* Public */,
       9,    1,   76,    2, 0x06 /* Public */,
      10,    1,   79,    2, 0x06 /* Public */,
      11,    1,   82,    2, 0x06 /* Public */,
      12,    1,   85,    2, 0x06 /* Public */,
      13,    1,   88,    2, 0x06 /* Public */,
      14,    2,   91,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   15,   16,

       0        // eod
};

void DlgAboutInitializer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DlgAboutInitializer *_t = static_cast<DlgAboutInitializer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->init_progress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->finished(); break;
        case 2: _t->got_p2p_version((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->got_chrome_version((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->got_rh_version((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->got_rh_management_version((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->got_x2go_version((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->got_vagrant_version((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: _t->got_oracle_virtualbox_version((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: _t->update_available((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (DlgAboutInitializer::*_t)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::init_progress)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::finished)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::got_p2p_version)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::got_chrome_version)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::got_rh_version)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::got_rh_management_version)) {
                *result = 5;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::got_x2go_version)) {
                *result = 6;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::got_vagrant_version)) {
                *result = 7;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::got_oracle_virtualbox_version)) {
                *result = 8;
                return;
            }
        }
        {
            typedef void (DlgAboutInitializer::*_t)(QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DlgAboutInitializer::update_available)) {
                *result = 9;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DlgAboutInitializer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DlgAboutInitializer.data,
      qt_meta_data_DlgAboutInitializer,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DlgAboutInitializer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DlgAboutInitializer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DlgAboutInitializer.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DlgAboutInitializer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void DlgAboutInitializer::init_progress(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DlgAboutInitializer::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DlgAboutInitializer::got_p2p_version(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DlgAboutInitializer::got_chrome_version(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DlgAboutInitializer::got_rh_version(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DlgAboutInitializer::got_rh_management_version(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void DlgAboutInitializer::got_x2go_version(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void DlgAboutInitializer::got_vagrant_version(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void DlgAboutInitializer::got_oracle_virtualbox_version(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void DlgAboutInitializer::update_available(QString _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}
struct qt_meta_stringdata_DlgAbout_t {
    QByteArrayData data[32];
    char stringdata0[573];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DlgAbout_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DlgAbout_t qt_meta_stringdata_DlgAbout = {
    {
QT_MOC_LITERAL(0, 0, 8), // "DlgAbout"
QT_MOC_LITERAL(1, 9, 24), // "btn_tray_update_released"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 23), // "btn_p2p_update_released"
QT_MOC_LITERAL(4, 59, 22), // "btn_rh_update_released"
QT_MOC_LITERAL(5, 82, 23), // "btn_rhm_update_released"
QT_MOC_LITERAL(6, 106, 24), // "btn_x2go_update_released"
QT_MOC_LITERAL(7, 131, 27), // "btn_vagrant_update_released"
QT_MOC_LITERAL(8, 159, 37), // "btn_oracle_virtualbox_update_..."
QT_MOC_LITERAL(9, 197, 20), // "btn_recheck_released"
QT_MOC_LITERAL(10, 218, 17), // "download_progress"
QT_MOC_LITERAL(11, 236, 7), // "file_id"
QT_MOC_LITERAL(12, 244, 3), // "rec"
QT_MOC_LITERAL(13, 248, 5), // "total"
QT_MOC_LITERAL(14, 254, 16), // "update_available"
QT_MOC_LITERAL(15, 271, 15), // "update_finished"
QT_MOC_LITERAL(16, 287, 7), // "success"
QT_MOC_LITERAL(17, 295, 16), // "install_finished"
QT_MOC_LITERAL(18, 312, 23), // "initialization_finished"
QT_MOC_LITERAL(19, 336, 16), // "init_progress_sl"
QT_MOC_LITERAL(20, 353, 4), // "part"
QT_MOC_LITERAL(21, 358, 18), // "got_p2p_version_sl"
QT_MOC_LITERAL(22, 377, 7), // "version"
QT_MOC_LITERAL(23, 385, 21), // "got_chrome_version_sl"
QT_MOC_LITERAL(24, 407, 17), // "got_rh_version_sl"
QT_MOC_LITERAL(25, 425, 28), // "got_rh_management_version_sl"
QT_MOC_LITERAL(26, 454, 19), // "got_x2go_version_sl"
QT_MOC_LITERAL(27, 474, 22), // "got_vagrant_version_sl"
QT_MOC_LITERAL(28, 497, 32), // "got_oracle_virtualbox_version_sl"
QT_MOC_LITERAL(29, 530, 19), // "update_available_sl"
QT_MOC_LITERAL(30, 550, 12), // "component_id"
QT_MOC_LITERAL(31, 563, 9) // "available"

    },
    "DlgAbout\0btn_tray_update_released\0\0"
    "btn_p2p_update_released\0btn_rh_update_released\0"
    "btn_rhm_update_released\0"
    "btn_x2go_update_released\0"
    "btn_vagrant_update_released\0"
    "btn_oracle_virtualbox_update_released\0"
    "btn_recheck_released\0download_progress\0"
    "file_id\0rec\0total\0update_available\0"
    "update_finished\0success\0install_finished\0"
    "initialization_finished\0init_progress_sl\0"
    "part\0got_p2p_version_sl\0version\0"
    "got_chrome_version_sl\0got_rh_version_sl\0"
    "got_rh_management_version_sl\0"
    "got_x2go_version_sl\0got_vagrant_version_sl\0"
    "got_oracle_virtualbox_version_sl\0"
    "update_available_sl\0component_id\0"
    "available"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DlgAbout[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  124,    2, 0x08 /* Private */,
       3,    0,  125,    2, 0x08 /* Private */,
       4,    0,  126,    2, 0x08 /* Private */,
       5,    0,  127,    2, 0x08 /* Private */,
       6,    0,  128,    2, 0x08 /* Private */,
       7,    0,  129,    2, 0x08 /* Private */,
       8,    0,  130,    2, 0x08 /* Private */,
       9,    0,  131,    2, 0x08 /* Private */,
      10,    3,  132,    2, 0x08 /* Private */,
      14,    1,  139,    2, 0x08 /* Private */,
      15,    2,  142,    2, 0x08 /* Private */,
      17,    2,  147,    2, 0x08 /* Private */,
      18,    0,  152,    2, 0x08 /* Private */,
      19,    2,  153,    2, 0x08 /* Private */,
      21,    1,  158,    2, 0x08 /* Private */,
      23,    1,  161,    2, 0x08 /* Private */,
      24,    1,  164,    2, 0x08 /* Private */,
      25,    1,  167,    2, 0x08 /* Private */,
      26,    1,  170,    2, 0x08 /* Private */,
      27,    1,  173,    2, 0x08 /* Private */,
      28,    1,  176,    2, 0x08 /* Private */,
      29,    2,  179,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong, QMetaType::LongLong,   11,   12,   13,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   11,   16,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   11,   16,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   20,   13,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   30,   31,

       0        // eod
};

void DlgAbout::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DlgAbout *_t = static_cast<DlgAbout *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->btn_tray_update_released(); break;
        case 1: _t->btn_p2p_update_released(); break;
        case 2: _t->btn_rh_update_released(); break;
        case 3: _t->btn_rhm_update_released(); break;
        case 4: _t->btn_x2go_update_released(); break;
        case 5: _t->btn_vagrant_update_released(); break;
        case 6: _t->btn_oracle_virtualbox_update_released(); break;
        case 7: _t->btn_recheck_released(); break;
        case 8: _t->download_progress((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2])),(*reinterpret_cast< qint64(*)>(_a[3]))); break;
        case 9: _t->update_available((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->update_finished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 11: _t->install_finished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 12: _t->initialization_finished(); break;
        case 13: _t->init_progress_sl((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 14: _t->got_p2p_version_sl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 15: _t->got_chrome_version_sl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 16: _t->got_rh_version_sl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 17: _t->got_rh_management_version_sl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 18: _t->got_x2go_version_sl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 19: _t->got_vagrant_version_sl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 20: _t->got_oracle_virtualbox_version_sl((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 21: _t->update_available_sl((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DlgAbout::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgAbout.data,
      qt_meta_data_DlgAbout,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DlgAbout::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DlgAbout::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DlgAbout.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int DlgAbout::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 22;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
