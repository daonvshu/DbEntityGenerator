#include "Generator.h"
#include <qfile.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qmap.h>
#include <qdebug.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qcryptographichash.h>

QList<Generator::Field> Generator::fieldList;
bool Generator::generatorStart(const QString & xmlPath) {
    qDebug() << xmlPath;
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
    auto isMysql = isMysqlDatabase(filePathBase);
	for (int i = 0; i < tbs.count(); i++) {
		auto tb = tbs.at(i).toElement();
		auto tbName = tb.attribute("name");
        if (tbName.isEmpty())
            continue;
		entityNames << tbName;

		QString engine = "";
		if (tb.hasAttribute("engine")) {
			engine = tb.attribute("engine");
		}

		auto hpp = hppTp;
		hpp.replace("$ClassName$", tbName);

		fieldList.clear();
		auto fields = tb.childNodes();
		for (int j = 0; j < fields.count(); j++) {
			auto fd = fields.at(j).toElement();

			Field field;
			field.name = fd.attribute("name");
			field.type = fd.attribute("qtype");
			field.index = fd.attribute("key") == "index";
			field.id = fd.attribute("key") == "id";
			field.attr = fd.text();
			field.note = fd.attribute("note");
            field.jsonField = fd.attribute("jfield", field.name);

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
		//引擎类型
		hpp.replace("$EngineType$", engine);
		//表名
		hpp.replace("$TbName$", prefix + tbName.toLower());
		//字段名列表
		hpp.replace("$FieldList$", getFieldListStr());
		//字段类型列表
		hpp.replace("$FieldType$", getFieldTypeStr(isMysql));
		//索引列表
		hpp.replace("$FieldIndex$", getIndexStr());
		//id
		hpp.replace("$FieldId$", getIdField());
		//id type
		hpp.replace("$IdType$", getIdType());
        //json to entity
        hpp.replace("$JsonToEntity$", getJson2EntityStr());
        //entity to json
        hpp.replace("$EntityToJson$", getEntity2JsonStr());
		//取值列表
		hpp.replace("$ReadEntity$", getReadEntityStr());
		//getter setter
		hpp.replace("$MemberGetterSetter$", getGetterSetterStr());
		//bind id
		hpp.replace("$BindId$", getBindIdStr());
		//bindvalue
		hpp.replace("$BindValues$", getBindValueStr());

		QFile hppFile(filePathBase + "/" + tbName + ".h");

		QStringList customCodeList;
		QString lineBuffer;

        QByteArray existContentHash;
		if (hppFile.exists()) {
            if (hppFile.open(QIODevice::ReadOnly)) {
                QTextStream in(&hppFile);
                bool customStart = false;
                QByteArray hppData;
                while (!in.atEnd()) {
                    lineBuffer = in.readLine();
                    hppData.append(lineBuffer);
                    hppData.append('\n');
                    if (lineBuffer.contains("CustomCodeArea")) {
                        customStart = true;
                        continue;
                    } else if (lineBuffer.contains("End")) {
                        customStart = false;
                    }
                    if (customStart) {
                        customCodeList.append(lineBuffer);
                    }
                }

                hppData = QString(hppData).replace(QRegExp("\\s"), "").toUtf8();
                existContentHash = QCryptographicHash::hash(hppData, QCryptographicHash::Md5);

                hppFile.close();
            }
		}
		lineBuffer = "";
		for (const auto& line : customCodeList) {
			lineBuffer.append(line).append('\n');
		}
		if (!lineBuffer.isEmpty()) {
			lineBuffer = lineBuffer.left(lineBuffer.length() - 1);
		}
		hpp.replace("$CustomCode$", lineBuffer);

        auto hppCotent = QString(hpp).replace(QRegExp("\\s"), "").toUtf8();
        auto currentHash = QCryptographicHash::hash(hppCotent, QCryptographicHash::Md5);
        if (existContentHash.compare(currentHash) != 0) {
            hppFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
            hppFile.write(hpp.toLocal8Bit());
            hppFile.close();
        }
	}
	
	QString includeContent = "#pragma once\n\n";
    QString entityNamesStr = "";
	for (const auto& n : entityNames) {
		includeContent.append(QString("#include \"%1.h\"\n").arg(n));
        entityNamesStr.append(n).append(", ");
	}
    entityNamesStr = entityNamesStr.mid(0, entityNamesStr.length() - 2);
    includeContent.append("\n#include \"../DaoUtil.h\"\n\n");
    includeContent.append("static void DbTablesInit(bool& success) {\n\tDbCreator<" + entityNamesStr + ">::init(success);\n}\n");

	QFile includeFile(filePathBase + "/DaoInclude.h");
    QByteArray includeHash;
    if (includeFile.exists()) {
        if (includeFile.open(QIODevice::ReadOnly)) {
            auto c = includeFile.readAll();
            c = QString(c).replace(QRegExp("\\s"), "").toUtf8();
            includeHash = QCryptographicHash::hash(c, QCryptographicHash::Md5);
            includeFile.close();
        }
    }
    auto ic = QString(includeContent).replace(QRegExp("\\s"), "").toUtf8();
    auto includeContentHash = QCryptographicHash::hash(ic, QCryptographicHash::Md5);
    if (includeHash.compare(includeContentHash) != 0) {
        includeFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        includeFile.write(includeContent.toLocal8Bit());
        includeFile.close();
    }

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
	QString fieldT = "\t\tEntityField %1 = \"%2\";\n";
	QString fieldStr;

	for (const auto& field : fieldList) {
		fieldStr.append(fieldT.arg(field.name, lowerAndSplitWithUnderline(field.name)));
	}

	return fieldStr;
}

QString Generator::getJoinBindFieldStr() {
	QString fieldStr;
	for (const auto& field : fieldList) {
		fieldStr.append(", ").append(field.name);
	}
	return fieldStr.mid(2);
}

QString Generator::getFieldListStr() {
	QString lt = "\n\t\t\t<< fields.%1()";
	QString fieldListStr;

	for (const auto& field : fieldList) {
		fieldListStr.append(lt.arg(field.name));
	}

	return fieldListStr;
}

QString Generator::getFieldTypeStr(bool isMysql) {
	QString lt = "\n\t\t<< QStringLiteral(\"%1 %2%3\")";
	QString fieldTypeStr;

	for (const auto& field : fieldList) {
		fieldTypeStr.append(lt.arg(lowerAndSplitWithUnderline(field.name), field.attr, (!isMysql || field.note.isEmpty()) ? "" : (" comment '" + field.note + "'")));
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

QString Generator::getIdField() {
	for (const auto& field : fieldList) {
		if (field.id) {
			return QString("\"%1\"").arg(field.name);
		}
	}
	return "\"\"";
}

QString Generator::getIdType() {
	for (const auto& field : fieldList) {
		if (field.id) {
			return field.type;
		}
	}
	return "int";
}

QString Generator::getJson2EntityStr() {
    static QMap<QString, QString> castType;
    if (castType.isEmpty()) {
        castType.insert("long", "Variant().toLongLong");
        castType.insert("qint64", "Variant().toLongLong");
        castType.insert("QString", "String");
        castType.insert("int", "Int");
        castType.insert("double", "Double");
        castType.insert("float", "Double");
        castType.insert("bool", "Int");
        //castType.insert("QByteArray", "Variant().toByteArray");
    }
    QString tp = "\t\t%1 = object.value(\"%2\").to%3();\n";
    QString json2EntityStr;

    for (const auto& field : fieldList) {
        if (field.type == "QByteArray") {
            json2EntityStr.append(QString("\t\t%1 = QByteArray::fromBase64(object.value(\"%2\").toString().toUtf8());\n").arg(field.name, field.jsonField));
            continue;
        }
        json2EntityStr.append(tp.arg(field.name, field.jsonField, (castType.contains(field.type) ? castType.value(field.type) : "unknown")));
    }

    return json2EntityStr;
}

QString Generator::getEntity2JsonStr() {
    QString tp = "\t\tobject.insert(\"%1\", %2);\n";
    QString entity2JsonStr;

    for (const auto& field : fieldList) {
        if (field.type == "QByteArray") {
            entity2JsonStr.append(tp.arg(field.jsonField, QString("QString(%1.toBase64())").arg(field.name)));
            continue;
        }
        entity2JsonStr.append(tp.arg(field.jsonField, (field.type == "bool" ? "(int)" : "") + field.name));
    }

    return entity2JsonStr;
}

QString Generator::getReadEntityStr() {
	QString lt = "\n\t\t\t<< get%1()";
	QString readEntityStr;

	for (const auto& field : fieldList) {
		if (field.id) {
			readEntityStr.append("\n\t\t\t<< QVariant()");
		} else {
			auto s1 = upperFirstChar(field.name);
			readEntityStr.append(lt.arg(QString(s1)));
		}
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
		if (field.id) {
			bindIdStr = QString("\t\tset%1(id);").arg(upperFirstChar(field.name));
			break;
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

	QString tp = "if (field == fields.%1()) {\n\t\t\tset%2(data%3);\n\t\t}";
	QString bindStr;

	for (const auto& field : fieldList) {
		if (!bindStr.isEmpty()) {
			bindStr.append(" else ");
		}
		auto s2 = upperFirstChar(field.name);
        if (field.type == "QVariant") {
            bindStr.append(tp.arg(field.name, s2, ""));
        } else {
            bindStr.append(tp.arg(field.name, s2, ".to" + (castType.contains(field.type) ? castType.value(field.type) : "unknown") + "()"));
        }
	}
    bindStr.append(" else {\n\t\t\textraData.insert(field, data);\n\t\t}");

	return bindStr;
}

QString Generator::upperFirstChar(const QString& s) {
	auto p1data = s.toLatin1();
	char p1char = p1data.at(0);
	return (char)(p1char - 32) + p1data.mid(1);
}

QString Generator::lowerAndSplitWithUnderline(const QString& s) {
	QByteArray p1data = s.toLatin1();
	QString result = "";
	int i = 0;
	int lastIndex = 0;
	for (; i < p1data.size(); i++) {
		char p1char = p1data.at(i);
		if (p1char >= 65 && p1char <= 90) {
			result += p1data.mid(lastIndex, i - lastIndex).toLower() + '_';
			lastIndex = i;
		}
	}
	result += p1data.mid(lastIndex).toLower();
	return result;
}

bool Generator::isMysqlDatabase(const QString & xmlPath) {
    QDir dir(xmlPath);
    dir.cdUp();
    auto xmlFiles = dir.entryInfoList(QStringList() << "*.xml");
    for (auto info : xmlFiles) {
        if (info.baseName() == "dao_cfg") {
            QFile file(info.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly)) {
                QDomDocument doc;
                if (doc.setContent(&file)) {
                    auto root = doc.documentElement();
                    auto client = root.attribute("type");
                    if (client == "mysql") {
                        file.close();
                        return true;
                    }
                }
                file.close();
            }
        }
    }
    return false;
}
