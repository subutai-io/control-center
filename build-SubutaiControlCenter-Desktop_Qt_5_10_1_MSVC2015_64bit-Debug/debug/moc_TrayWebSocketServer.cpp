/****************************************************************************
** Meta object code from reading C++ file 'TrayWebSocketServer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/TrayWebSocketServer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrayWebSocketServer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CTrayServer_t {
    QByteArrayData data[15];
    char stringdata0[203];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CTrayServer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CTrayServer_t qt_meta_stringdata_CTrayServer = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CTrayServer"
QT_MOC_LITERAL(1, 12, 17), // "on_new_connection"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 16), // "process_text_msg"
QT_MOC_LITERAL(4, 48, 3), // "msg"
QT_MOC_LITERAL(5, 52, 15), // "process_bin_msg"
QT_MOC_LITERAL(6, 68, 19), // "socket_disconnected"
QT_MOC_LITERAL(7, 88, 25), // "ssh_to_container_finished"
QT_MOC_LITERAL(8, 114, 12), // "CEnvironment"
QT_MOC_LITERAL(9, 127, 3), // "env"
QT_MOC_LITERAL(10, 131, 13), // "CHubContainer"
QT_MOC_LITERAL(11, 145, 4), // "cont"
QT_MOC_LITERAL(12, 150, 6), // "result"
QT_MOC_LITERAL(13, 157, 15), // "additional_data"
QT_MOC_LITERAL(14, 173, 29) // "desktop_to_container_finished"

    },
    "CTrayServer\0on_new_connection\0\0"
    "process_text_msg\0msg\0process_bin_msg\0"
    "socket_disconnected\0ssh_to_container_finished\0"
    "CEnvironment\0env\0CHubContainer\0cont\0"
    "result\0additional_data\0"
    "desktop_to_container_finished"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CTrayServer[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x08 /* Private */,
       3,    1,   45,    2, 0x08 /* Private */,
       5,    1,   48,    2, 0x08 /* Private */,
       6,    0,   51,    2, 0x08 /* Private */,
       7,    4,   52,    2, 0x0a /* Public */,
      14,    4,   61,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::QByteArray,    4,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8, 0x80000000 | 10, QMetaType::Int, QMetaType::VoidStar,    9,   11,   12,   13,
    QMetaType::Void, 0x80000000 | 8, 0x80000000 | 10, QMetaType::Int, QMetaType::VoidStar,    9,   11,   12,   13,

       0        // eod
};

void CTrayServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CTrayServer *_t = static_cast<CTrayServer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_new_connection(); break;
        case 1: _t->process_text_msg((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->process_bin_msg((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 3: _t->socket_disconnected(); break;
        case 4: _t->ssh_to_container_finished((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4]))); break;
        case 5: _t->desktop_to_container_finished((*reinterpret_cast< const CEnvironment(*)>(_a[1])),(*reinterpret_cast< const CHubContainer(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< void*(*)>(_a[4]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CTrayServer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CTrayServer.data,
      qt_meta_data_CTrayServer,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CTrayServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CTrayServer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CTrayServer.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CTrayServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
