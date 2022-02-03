#pragma once

#include <qstring.h>
#include <qlist.h>

struct Field {
	QString name; //require
	QString type; //require
	QString note;
	QString constraint;
	QString defaultValue;
	bool autoincreament;
	bool transient;
	QString jsonKey;
	QString jsonTimeFormat;
	//extra
	int bitsize;
	int decimal_d;
};

struct Index {
	QString indexType;
	QStringList fields;
	QString indexOptions;
};

struct Table {
    QString name;
	bool metatype; 
	QList<Field> fields;
	QList<Index> indexes;
	QList<QStringList> customConstructor;
	//extra
	QString engine;
};

struct Entity {
    QString prefix;
    QList<Table> tables;
};