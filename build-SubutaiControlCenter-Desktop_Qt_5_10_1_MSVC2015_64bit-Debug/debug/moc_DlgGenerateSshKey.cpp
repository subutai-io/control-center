/****************************************************************************
** Meta object code from reading C++ file 'DlgGenerateSshKey.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/DlgGenerateSshKey.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgGenerateSshKey.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DlgGenerateSshKey_t {
    QByteArrayData data[18];
    char stringdata0[279];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DlgGenerateSshKey_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DlgGenerateSshKey_t qt_meta_stringdata_DlgGenerateSshKey = {
    {
QT_MOC_LITERAL(0, 0, 17), // "DlgGenerateSshKey"
QT_MOC_LITERAL(1, 18, 21), // "btn_generate_released"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 24), // "btn_send_to_hub_released"
QT_MOC_LITERAL(4, 66, 25), // "environments_item_changed"
QT_MOC_LITERAL(5, 92, 14), // "QStandardItem*"
QT_MOC_LITERAL(6, 107, 4), // "item"
QT_MOC_LITERAL(7, 112, 25), // "lstv_keys_current_changed"
QT_MOC_LITERAL(8, 138, 3), // "ix0"
QT_MOC_LITERAL(9, 142, 3), // "ix1"
QT_MOC_LITERAL(10, 146, 24), // "ssh_key_send_progress_sl"
QT_MOC_LITERAL(11, 171, 4), // "part"
QT_MOC_LITERAL(12, 176, 5), // "total"
QT_MOC_LITERAL(13, 182, 24), // "ssh_key_send_finished_sl"
QT_MOC_LITERAL(14, 207, 30), // "chk_select_all_checked_changed"
QT_MOC_LITERAL(15, 238, 2), // "st"
QT_MOC_LITERAL(16, 241, 19), // "matrix_updated_slot"
QT_MOC_LITERAL(17, 261, 17) // "keys_updated_slot"

    },
    "DlgGenerateSshKey\0btn_generate_released\0"
    "\0btn_send_to_hub_released\0"
    "environments_item_changed\0QStandardItem*\0"
    "item\0lstv_keys_current_changed\0ix0\0"
    "ix1\0ssh_key_send_progress_sl\0part\0"
    "total\0ssh_key_send_finished_sl\0"
    "chk_select_all_checked_changed\0st\0"
    "matrix_updated_slot\0keys_updated_slot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DlgGenerateSshKey[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x08 /* Private */,
       3,    0,   60,    2, 0x08 /* Private */,
       4,    1,   61,    2, 0x08 /* Private */,
       7,    2,   64,    2, 0x08 /* Private */,
      10,    2,   69,    2, 0x08 /* Private */,
      13,    0,   74,    2, 0x08 /* Private */,
      14,    1,   75,    2, 0x08 /* Private */,
      16,    0,   78,    2, 0x08 /* Private */,
      17,    0,   79,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::QModelIndex, QMetaType::QModelIndex,    8,    9,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   11,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void DlgGenerateSshKey::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DlgGenerateSshKey *_t = static_cast<DlgGenerateSshKey *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->btn_generate_released(); break;
        case 1: _t->btn_send_to_hub_released(); break;
        case 2: _t->environments_item_changed((*reinterpret_cast< QStandardItem*(*)>(_a[1]))); break;
        case 3: _t->lstv_keys_current_changed((*reinterpret_cast< QModelIndex(*)>(_a[1])),(*reinterpret_cast< QModelIndex(*)>(_a[2]))); break;
        case 4: _t->ssh_key_send_progress_sl((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->ssh_key_send_finished_sl(); break;
        case 6: _t->chk_select_all_checked_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->matrix_updated_slot(); break;
        case 8: _t->keys_updated_slot(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DlgGenerateSshKey::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgGenerateSshKey.data,
      qt_meta_data_DlgGenerateSshKey,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DlgGenerateSshKey::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DlgGenerateSshKey::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DlgGenerateSshKey.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int DlgGenerateSshKey::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
