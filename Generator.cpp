#include "Generator.h"

#include "ConfigLoader.h"

#include "SqliteGenerator.h"

bool Generator::generatorStart(const QString & xmlPath) {
    
	ConfigLoader loader(xmlPath);
	if (!loader.load()) {
		return false;
	}

	switch (loader.getSqlType()) {
	case TYPE_SQLITE:
		SqliteGenerator generator(loader.getCfgFilePath(), loader.getEntity<SqliteEntity>());
		generator.generate();
		break;
	}

	return true;
}
