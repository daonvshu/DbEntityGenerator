#include <QtCore/QCoreApplication>
#include "Generator.h"
#include <qdebug.h>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QStringList arguments = QCoreApplication::arguments();
	if (arguments.size() < 2) {
		qDebug() << "input argument!";
		getchar();
	} else {
		if (!Generator::generatorStart(arguments.at(1))) {
			getchar();
		}
	}

	return 0;
}
