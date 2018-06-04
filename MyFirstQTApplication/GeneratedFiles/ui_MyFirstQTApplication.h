/********************************************************************************
** Form generated from reading UI file 'MyFirstQTApplication.ui'
**
** Created by: Qt User Interface Compiler version 5.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYFIRSTQTAPPLICATION_H
#define UI_MYFIRSTQTAPPLICATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MyFirstQTApplicationClass
{
public:
    QWidget *centralWidget;
    QPushButton *pushButton;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MyFirstQTApplicationClass)
    {
        if (MyFirstQTApplicationClass->objectName().isEmpty())
            MyFirstQTApplicationClass->setObjectName(QStringLiteral("MyFirstQTApplicationClass"));
        MyFirstQTApplicationClass->resize(600, 400);
        centralWidget = new QWidget(MyFirstQTApplicationClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(250, 160, 75, 23));
        MyFirstQTApplicationClass->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(MyFirstQTApplicationClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MyFirstQTApplicationClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MyFirstQTApplicationClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MyFirstQTApplicationClass->setStatusBar(statusBar);

        retranslateUi(MyFirstQTApplicationClass);
        QObject::connect(pushButton, SIGNAL(clicked()), MyFirstQTApplicationClass, SLOT(OnPushButtonClicked()));

        QMetaObject::connectSlotsByName(MyFirstQTApplicationClass);
    } // setupUi

    void retranslateUi(QMainWindow *MyFirstQTApplicationClass)
    {
        MyFirstQTApplicationClass->setWindowTitle(QApplication::translate("MyFirstQTApplicationClass", "MyFirstQTApplication", Q_NULLPTR));
        pushButton->setText(QApplication::translate("MyFirstQTApplicationClass", "PushButton", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MyFirstQTApplicationClass: public Ui_MyFirstQTApplicationClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYFIRSTQTAPPLICATION_H
