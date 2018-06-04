#include "MyFirstQTApplication.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MyFirstQTApplication w;
	w.show();
	return a.exec();
}
