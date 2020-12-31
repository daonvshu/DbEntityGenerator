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
				field.default = itemNode.attribute("default");
				field.autoincreament = QVariant(itemNode.attribute("autoincrement", "0")).toBool();
				field.transient = QVariant(itemNode.attribute("transient", "0")).toBool();
				//extra
				field.bitsize = QVariant(itemNode.attribute("bitsize", "0")).toInt();
				field.decimal_d = QVariant(itemNode.attribute("decimal-d", "0")).toInt();

				tableData.fields << field;
			} else if (itemNode.nodeName() == "index") {
				Index index;
				index.indexType = itemNode.attribute("type", "index");
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
			}
		}
		data->tables << tableData;
	}

	return true;
}
