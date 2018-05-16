/****************************************************************************
** Meta object code from reading C++ file 'NotificationObserver.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/NotificationObserver.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NotificationObserver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CNotificationObserver_t {
    QByteArrayData data[5];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CNotificationObserver_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CNotificationObserver_t qt_meta_stringdata_CNotificationObserver = {
    {
QT_MOC_LITERAL(0, 0, 21), // "CNotificationObserver"
QT_MOC_LITERAL(1, 22, 6), // "notify"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 43), // "CNotificationObserver::notifi..."
QT_MOC_LITERAL(4, 74, 41) // "DlgNotification::NOTIFICATION..."

    },
    "CNotificationObserver\0notify\0\0"
    "CNotificationObserver::notification_level_t\0"
    "DlgNotification::NOTIFICATION_ACTION_TYPE"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CNotificationObserver[] = {

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
       1,    3,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString, 0x80000000 | 4,    2,    2,    2,

       0        // eod
};

void CNotificationObserver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CNotificationObserver *_t = static_cast<CNotificationObserver *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->notify((*reinterpret_cast< CNotificationObserver::notification_level_t(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< DlgNotification::NOTIFICATION_ACTION_TYPE(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (CNotificationObserver::*_t)(CNotificationObserver::notification_level_t , const QString & , DlgNotification::NOTIFICATION_ACTION_TYPE );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CNotificationObserver::notify)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CNotificationObserver::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CNotificationObserver.data,
      qt_meta_data_CNotificationObserver,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CNotificationObserver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CNotificationObserver::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CNotificationObserver.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CNotificationObserver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void CNotificationObserver::notify(CNotificationObserver::notification_level_t _t1, const QString & _t2, DlgNotification::NOTIFICATION_ACTION_TYPE _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
