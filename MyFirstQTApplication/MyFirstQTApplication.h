#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MyFirstQTApplication.h"

class MyFirstQTApplication : public QMainWindow
{
	Q_OBJECT

public:
	MyFirstQTApplication(QWidget *parent = Q_NULLPTR);

private:
	Ui::MyFirstQTApplicationClass ui;
};
