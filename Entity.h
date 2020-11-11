#pragma once

#include <qstring.h>
#include <qlist.h>

struct Field {
	QString name; //require
	QString type; //require
	QString note;
	QString constraint;
	QString default;
	bool autoincreament;
	bool transient;
};

struct Index {
	QString indexType;
	QStringList fields;
};

struct Table {
    QString name;
	bool metatype; 
	QList<Field> fields;
	QList<Index> indexes;
};

struct Entity {
    QString prefix;
    QList<Table> tables;
};

struct SqliteEntity : public Entity {
};