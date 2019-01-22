#pragma once

#include <qobject.h>

class Generator {
public:
	static bool generatorStart(const QString& xmlPath);

private:
	static QString loadTemplateFile(const QString& name);

	struct Field {
		QString name;
		QString type;
		bool index;
		bool id;
		QString attr;
		QString note;
	};

	static QString getMembersStr();
	static QString getConstructFieldsStr();
	static QString getConstructCommitStr();
	static QString getFieldStr();
	static QString getJoinBindFieldStr();
	static QString getFieldListStr();
	static QString getFieldTypeStr();
	static QString getIndexStr();
	static QString getIdField();
	static QString getIdType();
	static QString getReadEntityStr();
	static QString getGetterSetterStr();
	static QString getBindIdStr();
	static QString getBindValueStr();
	
	static QString upperFirstChar(const QString & s);
	static QString lowerAndSplitWithUnderline(const QString& s);

private:
	static QList<Field> fieldList;
};

