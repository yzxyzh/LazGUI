/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      29,   11,   11,   11, 0x08,
      51,   42,   11,   11, 0x08,
      82,   42,   11,   11, 0x08,
     113,   11,   11,   11, 0x08,
     151,   11,   11,   11, 0x08,
     192,  186,   11,   11, 0x08,
     233,   11,   11,   11, 0x08,
     262,   11,   11,   11, 0x08,
     295,   11,   11,   11, 0x08,
     339,  327,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0ShowInfomation()\0RemoveItem()\0"
    "position\0on_LowerThres_sliderMoved(int)\0"
    "on_UpperThres_sliderMoved(int)\0"
    "on_actionAddBaseImgAction_triggered()\0"
    "on_actionAddMaskAction_triggered()\0"
    "index\0on_AlgBrowser_doubleClicked(QModelIndex)\0"
    "on_AlgSelector_textChanged()\0"
    "on_actionPhaseScript_triggered()\0"
    "on_actionSaveScript_triggered()\0"
    "item,column\0"
    "on_ResourceManager_itemChanged(QTreeWidgetItem*,int)\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->ShowInfomation(); break;
        case 1: _t->RemoveItem(); break;
        case 2: _t->on_LowerThres_sliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->on_UpperThres_sliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->on_actionAddBaseImgAction_triggered(); break;
        case 5: _t->on_actionAddMaskAction_triggered(); break;
        case 6: _t->on_AlgBrowser_doubleClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 7: _t->on_AlgSelector_textChanged(); break;
        case 8: _t->on_actionPhaseScript_triggered(); break;
        case 9: _t->on_actionSaveScript_triggered(); break;
        case 10: _t->on_ResourceManager_itemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE