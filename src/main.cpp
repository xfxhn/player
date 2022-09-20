
#include <QApplication>
#include "openglWidget.h"



int main(int argc, char* argv[]) {



	QApplication a(argc, argv);
	OpenglWidget widget;


	widget.show();

	return QApplication::exec();

}