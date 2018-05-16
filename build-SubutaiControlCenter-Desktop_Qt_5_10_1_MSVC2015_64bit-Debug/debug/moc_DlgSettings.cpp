/****************************************************************************
** Meta object code from reading C++ file 'DlgSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../control-center/hub/include/DlgSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DlgSettings_t {
    QByteArrayData data[20];
    char stringdata0[472];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DlgSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DlgSettings_t qt_meta_stringdata_DlgSettings = {
    {
QT_MOC_LITERAL(0, 0, 11), // "DlgSettings"
QT_MOC_LITERAL(1, 12, 23), // "le_terminal_cmd_changed"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 15), // "btn_ok_released"
QT_MOC_LITERAL(4, 53, 19), // "btn_cancel_released"
QT_MOC_LITERAL(5, 73, 28), // "btn_p2p_file_dialog_released"
QT_MOC_LITERAL(6, 102, 24), // "btn_scp_command_released"
QT_MOC_LITERAL(7, 127, 24), // "btn_ssh_command_released"
QT_MOC_LITERAL(8, 152, 30), // "btn_virtualbox_command_release"
QT_MOC_LITERAL(9, 183, 31), // "btn_x2goclient_command_released"
QT_MOC_LITERAL(10, 215, 31), // "btn_ssh_keygen_command_released"
QT_MOC_LITERAL(11, 247, 25), // "btn_logs_storage_released"
QT_MOC_LITERAL(12, 273, 29), // "btn_ssh_keys_storage_released"
QT_MOC_LITERAL(13, 303, 28), // "btn_refresh_rh_list_released"
QT_MOC_LITERAL(14, 332, 28), // "btn_vagrant_command_released"
QT_MOC_LITERAL(15, 361, 29), // "refresh_rh_list_timer_timeout"
QT_MOC_LITERAL(16, 391, 34), // "lstv_resource_hosts_double_cl..."
QT_MOC_LITERAL(17, 426, 3), // "ix0"
QT_MOC_LITERAL(18, 430, 29), // "resource_host_list_updated_sl"
QT_MOC_LITERAL(19, 460, 11) // "has_changes"

    },
    "DlgSettings\0le_terminal_cmd_changed\0"
    "\0btn_ok_released\0btn_cancel_released\0"
    "btn_p2p_file_dialog_released\0"
    "btn_scp_command_released\0"
    "btn_ssh_command_released\0"
    "btn_virtualbox_command_release\0"
    "btn_x2goclient_command_released\0"
    "btn_ssh_keygen_command_released\0"
    "btn_logs_storage_released\0"
    "btn_ssh_keys_storage_released\0"
    "btn_refresh_rh_list_released\0"
    "btn_vagrant_command_released\0"
    "refresh_rh_list_timer_timeout\0"
    "lstv_resource_hosts_double_clicked\0"
    "ix0\0resource_host_list_updated_sl\0"
    "has_changes"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DlgSettings[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x08 /* Private */,
       3,    0,   95,    2, 0x08 /* Private */,
       4,    0,   96,    2, 0x08 /* Private */,
       5,    0,   97,    2, 0x08 /* Private */,
       6,    0,   98,    2, 0x08 /* Private */,
       7,    0,   99,    2, 0x08 /* Private */,
       8,    0,  100,    2, 0x08 /* Private */,
       9,    0,  101,    2, 0x08 /* Private */,
      10,    0,  102,    2, 0x08 /* Private */,
      11,    0,  103,    2, 0x08 /* Private */,
      12,    0,  104,    2, 0x08 /* Private */,
      13,    0,  105,    2, 0x08 /* Private */,
      14,    0,  106,    2, 0x08 /* Private */,
      15,    0,  107,    2, 0x08 /* Private */,
      16,    1,  108,    2, 0x08 /* Private */,
      18,    1,  111,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QModelIndex,   17,
    QMetaType::Void, QMetaType::Bool,   19,

       0        // eod
};

void DlgSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DlgSettings *_t = static_cast<DlgSettings *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->le_terminal_cmd_changed(); break;
        case 1: _t->btn_ok_released(); break;
        case 2: _t->btn_cancel_released(); break;
        case 3: _t->btn_p2p_file_dialog_released(); break;
        case 4: _t->btn_scp_command_released(); break;
        case 5: _t->btn_ssh_command_released(); break;
        case 6: _t->btn_virtualbox_command_release(); break;
        case 7: _t->btn_x2goclient_command_released(); break;
        case 8: _t->btn_ssh_keygen_command_released(); break;
        case 9: _t->btn_logs_storage_released(); break;
        case 10: _t->btn_ssh_keys_storage_released(); break;
        case 11: _t->btn_refresh_rh_list_released(); break;
        case 12: _t->btn_vagrant_command_released(); break;
        case 13: _t->refresh_rh_list_timer_timeout(); break;
        case 14: _t->lstv_resource_hosts_double_clicked((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 15: _t->resource_host_list_updated_sl((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DlgSettings::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgSettings.data,
      qt_meta_data_DlgSettings,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DlgSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DlgSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DlgSettings.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int DlgSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
