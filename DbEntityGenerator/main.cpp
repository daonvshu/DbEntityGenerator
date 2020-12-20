#include <QtCore/QCoreApplication>
#include "Generator.h"
#include <qdebug.h>

//#define DEBUG
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

#ifndef DEBUG
	QStringList arguments = QCoreApplication::arguments();
	if (arguments.size() < 2) {
		qDebug() << "input argument!";
		getchar();
	} else {
		if (!Generator::generatorStart(arguments.at(1))) {
			getchar();
		}
	}
#else
	if (!Generator::generatorStart("dao_entity.xml")) {
		getchar();
	}
#endif // !DEBUG

	return 0;
}
