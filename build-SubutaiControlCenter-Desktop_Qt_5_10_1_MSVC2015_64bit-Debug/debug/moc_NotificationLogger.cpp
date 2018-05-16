/****************************************************************************
** Meta object code from reading C++ file 'NotificationLogger.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/NotificationLogger.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NotificationLogger.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CNotificationLogger_t {
    QByteArrayData data[8];
    char stringdata0[139];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CNotificationLogger_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CNotificationLogger_t qt_meta_stringdata_CNotificationLogger = {
    {
QT_MOC_LITERAL(0, 0, 19), // "CNotificationLogger"
QT_MOC_LITERAL(1, 20, 21), // "notifications_updated"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 21), // "notification_received"
QT_MOC_LITERAL(4, 65, 43), // "CNotificationObserver::notifi..."
QT_MOC_LITERAL(5, 109, 5), // "level"
QT_MOC_LITERAL(6, 115, 3), // "str"
QT_MOC_LITERAL(7, 119, 19) // "clear_timer_timeout"

    },
    "CNotificationLogger\0notifications_updated\0"
    "\0notification_received\0"
    "CNotificationObserver::notification_level_t\0"
    "level\0str\0clear_timer_timeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CNotificationLogger[] = {

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
       1,    0,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    2,   30,    2, 0x08 /* Private */,
       7,    0,   35,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4, QMetaType::QString,    5,    6,
    QMetaType::Void,

       0        // eod
};

void CNotificationLogger::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CNotificationLogger *_t = static_cast<CNotificationLogger *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->notifications_updated(); break;
        case 1: _t->notification_received((*reinterpret_cast< CNotificationObserver::notification_level_t(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->clear_timer_timeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (CNotificationLogger::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CNotificationLogger::notifications_updated)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CNotificationLogger::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CNotificationLogger.data,
      qt_meta_data_CNotificationLogger,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CNotificationLogger::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CNotificationLogger::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CNotificationLogger.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CNotificationLogger::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void CNotificationLogger::notifications_updated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
