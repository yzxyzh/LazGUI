/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSlider>
#include <QtGui/QStatusBar>
#include <QtGui/QTableWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBar>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionPhaseScript;
    QAction *actionSaveScript;
    QWidget *centralWidget;
    QLabel *ImageViewer;
    QTreeWidget *ResourceManager;
    QSlider *LowerThres;
    QSlider *UpperThres;
    QTextEdit *cmdShow;
    QListWidget *AlgBrowser;
    QTextEdit *AlgSelector;
    QTableWidget *StackShow;
    QTableWidget *PropertiesShow;
    QTextEdit *StatusShow;
    QMenuBar *menuBar;
    QMenu *menuOptions;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1338, 784);
        actionPhaseScript = new QAction(MainWindow);
        actionPhaseScript->setObjectName(QString::fromUtf8("actionPhaseScript"));
        actionSaveScript = new QAction(MainWindow);
        actionSaveScript->setObjectName(QString::fromUtf8("actionSaveScript"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        ImageViewer = new QLabel(centralWidget);
        ImageViewer->setObjectName(QString::fromUtf8("ImageViewer"));
        ImageViewer->setGeometry(QRect(280, 20, 491, 491));
        ResourceManager = new QTreeWidget(centralWidget);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        ResourceManager->setHeaderItem(__qtreewidgetitem);
        ResourceManager->setObjectName(QString::fromUtf8("ResourceManager"));
        ResourceManager->setGeometry(QRect(790, 0, 541, 561));
        LowerThres = new QSlider(centralWidget);
        LowerThres->setObjectName(QString::fromUtf8("LowerThres"));
        LowerThres->setGeometry(QRect(270, 520, 501, 22));
        LowerThres->setOrientation(Qt::Horizontal);
        UpperThres = new QSlider(centralWidget);
        UpperThres->setObjectName(QString::fromUtf8("UpperThres"));
        UpperThres->setGeometry(QRect(270, 550, 501, 22));
        UpperThres->setOrientation(Qt::Horizontal);
        cmdShow = new QTextEdit(centralWidget);
        cmdShow->setObjectName(QString::fromUtf8("cmdShow"));
        cmdShow->setGeometry(QRect(10, 580, 371, 141));
        AlgBrowser = new QListWidget(centralWidget);
        AlgBrowser->setObjectName(QString::fromUtf8("AlgBrowser"));
        AlgBrowser->setGeometry(QRect(10, 80, 251, 481));
        AlgSelector = new QTextEdit(centralWidget);
        AlgSelector->setObjectName(QString::fromUtf8("AlgSelector"));
        AlgSelector->setGeometry(QRect(10, 30, 251, 41));
        StackShow = new QTableWidget(centralWidget);
        StackShow->setObjectName(QString::fromUtf8("StackShow"));
        StackShow->setGeometry(QRect(790, 580, 261, 141));
        PropertiesShow = new QTableWidget(centralWidget);
        PropertiesShow->setObjectName(QString::fromUtf8("PropertiesShow"));
        PropertiesShow->setGeometry(QRect(1060, 580, 271, 141));
        StatusShow = new QTextEdit(centralWidget);
        StatusShow->setObjectName(QString::fromUtf8("StatusShow"));
        StatusShow->setGeometry(QRect(400, 580, 371, 141));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1338, 22));
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName(QString::fromUtf8("menuOptions"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuOptions->menuAction());
        menuOptions->addAction(actionPhaseScript);
        menuOptions->addAction(actionSaveScript);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionPhaseScript->setText(QApplication::translate("MainWindow", "PhaseScript", 0, QApplication::UnicodeUTF8));
        actionSaveScript->setText(QApplication::translate("MainWindow", "SaveScript", 0, QApplication::UnicodeUTF8));
        ImageViewer->setText(QString());
        menuOptions->setTitle(QApplication::translate("MainWindow", "Options", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
