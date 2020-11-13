#include "Generator.h"

#include "ConfigLoader.h"

#include "SqliteGenerator.h"

bool Generator::generatorStart(const QString & xmlPath, const QString& dbloaderPath) {
    
	ConfigLoader loader(xmlPath);
	if (!loader.load()) {
		return false;
	}

	switch (loader.getSqlType()) {
	case TYPE_SQLITE:
		SqliteGenerator generator(loader.getCfgFilePath(), loader.getEntity<SqliteEntity>(), dbloaderPath);
		generator.generate();
		break;
	}

	return true;
}
