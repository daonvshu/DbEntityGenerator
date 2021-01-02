﻿#pragma once

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
	//extra
	int bitsize;
	int decimal_d;
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
	//extra
	QString engine;
};

struct Entity {
    QString prefix;
    QList<Table> tables;
};