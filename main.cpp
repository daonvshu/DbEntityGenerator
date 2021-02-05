#include <QtCore/QCoreApplication>
#include "Generator.h"
#include <qdebug.h>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

#ifndef QT_DEBUG
	QStringList arguments = QCoreApplication::arguments();
	if (arguments.size() < 2) {
		qDebug() << "input argument!";
		getchar();
	} else {
		if (!Generator::generatorStart(arguments.at(1), arguments.size() == 2 ? QString() : arguments.at(2))) {
			getchar();
		}
	}
#else
	if (!Generator::generatorStart("test/sqlserver_entity.xml", "")) {
		getchar();
	}
#endif // !DEBUG

	return 0;
}
