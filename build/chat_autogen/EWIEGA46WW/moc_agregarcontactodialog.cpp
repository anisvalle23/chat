/****************************************************************************
** Meta object code from reading C++ file 'agregarcontactodialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../agregarcontactodialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'agregarcontactodialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AgregarContactoDialog_t {
    QByteArrayData data[6];
    char stringdata0[82];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AgregarContactoDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AgregarContactoDialog_t qt_meta_stringdata_AgregarContactoDialog = {
    {
QT_MOC_LITERAL(0, 0, 21), // "AgregarContactoDialog"
QT_MOC_LITERAL(1, 22, 19), // "onSearchTextChanged"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 4), // "text"
QT_MOC_LITERAL(4, 48, 12), // "onAddClicked"
QT_MOC_LITERAL(5, 61, 20) // "updateSelectionStyle"

    },
    "AgregarContactoDialog\0onSearchTextChanged\0"
    "\0text\0onAddClicked\0updateSelectionStyle"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AgregarContactoDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x08 /* Private */,
       4,    0,   32,    2, 0x08 /* Private */,
       5,    0,   33,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void AgregarContactoDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AgregarContactoDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onSearchTextChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->onAddClicked(); break;
        case 2: _t->updateSelectionStyle(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AgregarContactoDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_AgregarContactoDialog.data,
    qt_meta_data_AgregarContactoDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AgregarContactoDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AgregarContactoDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AgregarContactoDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int AgregarContactoDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
