#include "Generator.h"
#include <qfile.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qmap.h>
#include <qdebug.h>

QList<Generator::Field> Generator::fieldList;
bool Generator::generatorStart(const QString & xmlPath) {
	QDomDocument doc;
	QFile file(xmlPath);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "open file fail!";
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
	auto filePathBase = fileInfo.absolutePath();
	qDebug() << "filebase:" << filePathBase;

	auto root = doc.documentElement();
	if (root.tagName() != "dao") {
		qDebug() << "the root name isnot dao!";
		return false;
	}

	QString hppTp = loadTemplateFile(":/generator/EntityTemplateHpp.txt");

	QString prefix = root.attribute("prefix");
	auto tbs = root.childNodes();
	QStringList entityNames;
	for (int i = 0; i < tbs.count(); i++) {
		auto tb = tbs.at(i).toElement();
		auto tbName = tb.attribute("name");
		entityNames << tbName;

		auto hpp = hppTp;
		hpp.replace("$ClassName$", tbName);

		fieldList.clear();
		auto fields = tb.childNodes();
		for (int j = 0; j < fields.count(); j++) {
			auto fd = fields.at(j).toElement();

			Field field;
			field.name = fd.attribute("name");
			field.type = fd.attribute("qtype");
			field.index = fd.attribute("index", "false") == "true";
			field.attr = fd.text();
			field.note = fd.attribute("note");

			fieldList.append(field);
		}
		//成员函数
		hpp.replace("$Members$", getMembersStr());
		//成员函数列表
		hpp.replace("$Fields$", getFieldStr());
		//join bind字段列表
		hpp.replace("$BindFieldList$", getJoinBindFieldStr());
		//构造函数参数
		hpp.replace("$ConstructFields$", getConstructFieldsStr());
		//构造函数赋值
		hpp.replace("$ConstructCommit$", getConstructCommitStr());
		//字段总数
		hpp.replace("$FieldSize$", QString::number(fieldList.size()));
		//表名
		hpp.replace("$TbName$", prefix + tbName.toLower());
		//字段名列表
		hpp.replace("$FieldList$", getFieldListStr());
		//字段类型列表
		hpp.replace("$FieldType$", getFieldTypeStr());
		//索引列表
		hpp.replace("$FieldIndex$", getIndexStr());
		//取值列表
		hpp.replace("$ReadEntity$", getReadEntityStr());
		//getter setter
		hpp.replace("$MemberGetterSetter$", getGetterSetterStr());
		//bind id
		hpp.replace("$BindId$", getBindIdStr());
		//bindvalue
		hpp.replace("$BindValues$", getBindValueStr());

		QFile hppFile(filePathBase + "/" + tbName + ".h");
		hppFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
		hppFile.write(hpp.toLocal8Bit());
		hppFile.close();
	}
	
	QString includeContent = "#pragma once\n\n";
	for (const auto& n : entityNames) {
		includeContent.append(QString("#include \"%1.h\"\n").arg(n));
	}
	QFile includeFile(filePathBase + "/DaoInclude.h");
	includeFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	includeFile.write(includeContent.toLocal8Bit());
	includeFile.close();

	return true;
}

QString Generator::loadTemplateFile(const QString & name) {
	QFile file(name);
	file.open(QIODevice::ReadOnly);
	auto data = file.readAll();
	file.close();
	return data;
}

QString Generator::getMembersStr() {
	QString member = "\t%1 %2;%3\n";
	QString memberStr;

	for (const auto& field : fieldList) {
		memberStr.append(member.arg(field.type, field.name, 
			field.note.isEmpty() ? "" : ("//" + field.note)));
	}

	return memberStr;
}

QString Generator::getConstructFieldsStr() {
	QString ft = "%1 %2";
	QString constructFieldsStr;

	for (const auto& field : fieldList) {
		auto s1 = field.type;
		char p2char = s1.toLatin1().at(0);
		if (p2char >= 65 && p2char <= 90) {
			s1 = QString("const %1&").arg(s1);
		}
		constructFieldsStr.append(ft.arg(s1, field.name));
		constructFieldsStr.append(", ");
	}
	constructFieldsStr = constructFieldsStr.left(constructFieldsStr.length() - 2);

	return constructFieldsStr;
}

QString Generator::getConstructCommitStr() {
	QString ft = "\tthis->%1 = %1;\n\t";
	QString commitStr;

	for (const auto& field : fieldList) {
		commitStr.append(ft.arg(field.name));
	}

	return commitStr;
}

QString Generator::getFieldStr() {
	QString fieldT = "\t\tDaoEntityField %1 = \"%1\"_field;\n";
	QString fieldStr;

	for (const auto& field : fieldList) {
		fieldStr.append(fieldT.arg(field.name));
	}

	return fieldStr;
}

QString Generator::getJoinBindFieldStr() {
	QString fieldStr;
	for (const auto& field : fieldList) {
		fieldStr.append(", ").append(field.name);
	}
	return fieldStr;
}

QString Generator::getFieldListStr() {
	QString lt = "\n\t\t\t<< fields.%1()";
	QString fieldListStr;

	for (const auto& field : fieldList) {
		fieldListStr.append(lt.arg(field.name));
	}

	return fieldListStr;
}

QString Generator::getFieldTypeStr() {
	QString lt = "\n\t\t<< QStringLiteral(\"%1 %2\")";
	QString fieldTypeStr;

	for (const auto& field : fieldList) {
		fieldTypeStr.append(lt.arg(field.name, field.attr));
	}

	return fieldTypeStr;
}

QString Generator::getIndexStr() {
	QString lt = "\n\t\t\t<< fields.%1()";
	QString fieldListStr;

	for (const auto& field : fieldList) {
		if (field.index) {
			fieldListStr.append(lt.arg(field.name));
		}
	}

	return fieldListStr;
}

QString Generator::getReadEntityStr() {
	QString lt = "\n\t\t\t<< get%1()";
	QString readEntityStr;

	for (const auto& field : fieldList) {
		auto s1 = upperFirstChar(field.name);
		readEntityStr.append(lt.arg(QString(s1)));
	}

	return readEntityStr;
}

QString Generator::getGetterSetterStr() {
	QString tp = "\tinline void set%1(%2 %3) {this->%3 = %3;}\n"
		"\tinline %4 get%1() const {return %3;}\n";
	QString getsetStr;

	for (const auto& field : fieldList) {
		auto s1 = upperFirstChar(field.name);
		auto s2 = field.type;
		char p2char = s2.toLatin1().at(0);
		if (p2char >= 65 && p2char <= 90) {
			s2 = QString("const %1&").arg(s2);
		}

		getsetStr.append(tp.arg(s1, s2, field.name, field.type));
	}

	return getsetStr;
}

QString Generator::getBindIdStr() {
	QString bindIdStr = "";
	for (const auto& field : fieldList) {
		if (field.name == "id") {
			bindIdStr = "\t\tsetId(id);";
		}
	}
	return bindIdStr;
}

QString Generator::getBindValueStr() {
	static QMap<QString, QString> castType;
	if (castType.isEmpty()) {
		castType.insert("long", "LongLong");
		castType.insert("qint64", "LongLong");
		castType.insert("QString", "String");
		castType.insert("int", "Int");
		castType.insert("double", "Double");
		castType.insert("float", "Float");
		castType.insert("bool", "Bool");
		castType.insert("QByteArray", "ByteArray");
	}

	QString tp = "if (field == fields.%1()) {\n\t\t\tset%2(data.to%3());\n\t\t}";
	QString bindStr;

	for (const auto& field : fieldList) {
		if (!bindStr.isEmpty()) {
			bindStr.append(" else ");
		}
		auto s2 = upperFirstChar(field.name);
		bindStr.append(tp.arg(field.name, s2, castType.contains(field.type) ? castType.value(field.type) : "unknown"));
	}

	return bindStr;
}

QString Generator::upperFirstChar(const QString& s) {
	auto p1data = s.toLatin1();
	char p1char = p1data.at(0);
	return (char)(p1char - 32) + p1data.mid(1);
}