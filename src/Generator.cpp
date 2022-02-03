#include "Generator.h"

#include "ConfigLoader.h"

#include "SqliteGenerator.h"
#include "MysqlGenerator.h"
#include "SqlServerGenerator.h"

bool Generator::generatorStart(const QString & xmlPath, const QString& dbloaderPath) {
    
	ConfigLoader loader(xmlPath);
	if (!loader.load()) {
		return false;
	}

	switch (loader.getSqlType()) {
	case TYPE_SQLITE:
		SqliteGenerator(loader.getCfgFilePath(), loader.getEntity(), dbloaderPath).generate();
		break;
	case TYPE_MYSQL:
		MysqlGenerator(loader.getCfgFilePath(), loader.getEntity(), dbloaderPath).generate();
		break;
	case TYPE_SQLSERVER:
		SqlServerGenerator(loader.getCfgFilePath(), loader.getEntity(), dbloaderPath).generate();
		break;
	}

	return true;
}
