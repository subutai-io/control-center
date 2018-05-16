/****************************************************************************
** Meta object code from reading C++ file 'IUpdaterComponent.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/updater/IUpdaterComponent.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IUpdaterComponent.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_update_system__IUpdaterComponent_t {
    QByteArrayData data[13];
    char stringdata0[198];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_update_system__IUpdaterComponent_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_update_system__IUpdaterComponent_t qt_meta_stringdata_update_system__IUpdaterComponent = {
    {
QT_MOC_LITERAL(0, 0, 32), // "update_system::IUpdaterComponent"
QT_MOC_LITERAL(1, 33, 15), // "update_progress"
QT_MOC_LITERAL(2, 49, 0), // ""
QT_MOC_LITERAL(3, 50, 12), // "component_id"
QT_MOC_LITERAL(4, 63, 4), // "part"
QT_MOC_LITERAL(5, 68, 5), // "total"
QT_MOC_LITERAL(6, 74, 15), // "update_finished"
QT_MOC_LITERAL(7, 90, 7), // "success"
QT_MOC_LITERAL(8, 98, 16), // "install_finished"
QT_MOC_LITERAL(9, 115, 24), // "update_available_changed"
QT_MOC_LITERAL(10, 140, 18), // "update_finished_sl"
QT_MOC_LITERAL(11, 159, 19), // "install_finished_sl"
QT_MOC_LITERAL(12, 179, 18) // "update_progress_sl"

    },
    "update_system::IUpdaterComponent\0"
    "update_progress\0\0component_id\0part\0"
    "total\0update_finished\0success\0"
    "install_finished\0update_available_changed\0"
    "update_finished_sl\0install_finished_sl\0"
    "update_progress_sl"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_update_system__IUpdaterComponent[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   49,    2, 0x06 /* Public */,
       6,    2,   56,    2, 0x06 /* Public */,
       8,    2,   61,    2, 0x06 /* Public */,
       9,    1,   66,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    1,   69,    2, 0x09 /* Protected */,
      11,    1,   72,    2, 0x09 /* Protected */,
      12,    2,   75,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong, QMetaType::LongLong,    3,    4,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    7,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    7,
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, QMetaType::LongLong, QMetaType::LongLong,    4,    5,

       0        // eod
};

void update_system::IUpdaterComponent::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        IUpdaterComponent *_t = static_cast<IUpdaterComponent *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->update_progress((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2])),(*reinterpret_cast< qint64(*)>(_a[3]))); break;
        case 1: _t->update_finished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->install_finished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->update_available_changed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->update_finished_sl((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->install_finished_sl((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->update_progress_sl((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (IUpdaterComponent::*_t)(const QString & , qint64 , qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IUpdaterComponent::update_progress)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (IUpdaterComponent::*_t)(const QString & , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IUpdaterComponent::update_finished)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (IUpdaterComponent::*_t)(const QString & , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IUpdaterComponent::install_finished)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (IUpdaterComponent::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IUpdaterComponent::update_available_changed)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject update_system::IUpdaterComponent::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_update_system__IUpdaterComponent.data,
      qt_meta_data_update_system__IUpdaterComponent,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *update_system::IUpdaterComponent::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *update_system::IUpdaterComponent::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_update_system__IUpdaterComponent.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int update_system::IUpdaterComponent::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void update_system::IUpdaterComponent::update_progress(const QString & _t1, qint64 _t2, qint64 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void update_system::IUpdaterComponent::update_finished(const QString & _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void update_system::IUpdaterComponent::install_finished(const QString & _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void update_system::IUpdaterComponent::update_available_changed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
