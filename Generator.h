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
        QString jsonField;
	};

	static QString getMembersStr();
	static QString getConstructFieldsStr();
	static QString getConstructCommitStr();
	static QString getFieldStr();
	static QString getJoinBindFieldStr();
	static QString getFieldListStr();
	static QString getFieldTypeStr(bool isMysql);
	static QString getIndexStr();
	static QString getIdField();
	static QString getIdType();
	static bool isIdInteger();
	static QString getIdFieldName();
    static QString getJson2EntityStr();
    static QString getEntity2JsonStr();
	static QString getReadEntityStr();
	static QString getGetterSetterStr();
	static QString getBindIdStr();
	static QString getBindValueStr();
	
	static QString upperFirstChar(const QString & s);
	static QString lowerAndSplitWithUnderline(const QString& s);
    static bool isMysqlDatabase(const QString& xmlPath);

private:
	static QList<Field> fieldList;
};

