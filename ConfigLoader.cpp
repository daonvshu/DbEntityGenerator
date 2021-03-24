#include "ConfigLoader.h"

#include <qdebug.h>
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>

ConfigLoader::ConfigLoader(const QString& configPath) {
    path = configPath;
	data = nullptr;
}

ConfigLoader::~ConfigLoader() {
	if (data) {
		delete data;
	}
}

bool ConfigLoader::load() {
	QDomDocument doc;
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "open file fail! path-> " << path;
		return false;
	}
	QString errMsg;
	int errLine;
	int errCol;
	if (!doc.setContent(&file, &errMsg, &errLine, &errCol)) {
		file.close();
		qDebug() << "load dom fail!err:" << errMsg << " line:" << errLine << " col:" << errCol;
		return false;
	}
	file.close();

	QFileInfo fileInfo(file);
	cfgFilePath = fileInfo.absolutePath();

	auto root = doc.documentElement();
	if (root.tagName() != "dao") {
		qDebug() << "the root name isnot dao!";
		return false;
	}

	QString prefix = root.attribute("prefix");
	QString dbType = root.attribute("db");

	data = new Entity;
	if (dbType == "sqlite") {
		configType = TYPE_SQLITE;
	} else if (dbType == "mysql") {
		configType = TYPE_MYSQL;
	} else if (dbType == "sqlserver") {
		configType = TYPE_SQLSERVER;
	} else {
		qDebug() << "unknown db type";
		return false;
	}
	data->prefix = prefix;

	auto tableNodes = root.childNodes();
	for (int i = 0; i < tableNodes.count(); i++) {
		Table tableData;
		auto tbNode = tableNodes.at(i).toElement();
		tableData.name = tbNode.attribute("name");
		if (tableData.name.isEmpty()) {
			continue;
		}
		tableData.metatype = QVariant(tbNode.attribute("declaremetatype", "0")).toBool();
		tableData.engine = QVariant(tbNode.attribute("engine")).toString();
		auto itemNodes = tbNode.childNodes();
		for (int j = 0; j < itemNodes.count(); j++) {
			auto itemNode = itemNodes.at(j).toElement();
			if (itemNode.nodeName() == "item") {
				Field field;
				field.name = itemNode.attribute("name");
				field.type = itemNode.attribute("type");
				field.note = itemNode.attribute("note");
				field.constraint = itemNode.attribute("constraints");
				field.defaultValue = itemNode.attribute("default");
				field.jsonKey = itemNode.attribute("jsonkey");
				field.jsonTimeFormat = itemNode.attribute("jsontimeformat");
				field.autoincreament = QVariant(itemNode.attribute("autoincrement", "0")).toBool();
				field.transient = QVariant(itemNode.attribute("transient", "0")).toBool();
				//extra
				field.bitsize = QVariant(itemNode.attribute("bitsize", "0")).toInt();
				field.decimal_d = QVariant(itemNode.attribute("decimal-d", "0")).toInt();

				tableData.fields << field;
			} else if (itemNode.nodeName() == "index") {
				Index index;
				if (configType != TYPE_SQLSERVER) {
					index.indexType = itemNode.attribute("type", "index");
				} else {
					index.indexType = itemNode.attribute("type", "nonclustered");
					auto optionIgnoreDupKey = itemNode.attribute("ignore_dup_key");
					if (!optionIgnoreDupKey.isEmpty()) {
						index.indexOptions.append("IGNORE_DUP_KEY=").append(optionIgnoreDupKey).append(',');
					}
					auto allowRowLocks = itemNode.attribute("allow_row_locks");
					if (!allowRowLocks.isEmpty()) {
						index.indexOptions.append("ALLOW_ROW_LOCKS=").append(allowRowLocks).append(',');
					}
					auto allowPageLocks = itemNode.attribute("allow_page_locks");
					if (!allowPageLocks.isEmpty()) {
						index.indexOptions.append("ALLOW_PAGE_LOCKS=").append(allowPageLocks).append(',');
					}
					auto dataCompression = itemNode.attribute("data_compression");
					if (!dataCompression.isEmpty()) {
						index.indexOptions.append("DATA_COMPRESSION=").append(dataCompression).append(',');
					}
					if (!index.indexOptions.isEmpty()) {
						index.indexOptions.chop(1);
					}
				}
				auto indexFieldNodes = itemNode.childNodes();
				for (int k = 0; k < indexFieldNodes.size(); k++) {
					auto ele = indexFieldNodes.at(k).toElement();
					QString seq = ele.attribute("seq");
					if (seq.isEmpty()) {
						index.fields << ele.text();
					} else {
						index.fields << ele.text() + ' ' + seq;
					}
				}
				tableData.indexes << index;
			} else if (itemNode.nodeName() == "constructor") {
				QStringList fields;
				auto fieldNodes = itemNode.childNodes();
				for (int k = 0; k < fieldNodes.size(); k++) {
					auto ele = fieldNodes.at(k).toElement();
					fields << ele.text();
				}
				tableData.customConstructor << fields;
			}
		}
		data->tables << tableData;
	}

	return true;
}
