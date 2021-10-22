#include "MysqlGenerator.h"

MysqlGenerator::MysqlGenerator(QString outputPath, Entity entity, QString dbloadPath)
    : AbstractGenerator(outputPath, dbloadPath)
    , entity(entity) {
}

void MysqlGenerator::generate() {
    QString hppTemplate = loadTemplateFile(":/generator/templates/mysql.txt");
    QStringList tbnames;
    for (const auto& tb : entity.tables) {
        setCurrentTable(tb);
        tbnames << tb.name;
        auto header = hppTemplate;
        //set entity path
        header.replace("$DbLoaderPath$", dbloadPath);
        //set classname
        header.replace("$ClassName$", tb.name);
        //set field list
        header.replace("$Members$", createFieldList());
        //set construct
        header.replace("$Construct$", createConstruct());
        //set field declare
        header.replace("$FieldDeclare$", createFieldDeclare(entity.prefix));
        header.replace("$FieldDeclareReset$", createFieldDeclareReset());
        //set field size
        header.replace("$FieldSize$", createFieldSize());
        //set tablename
        header.replace("$TbName$", createTableName(entity.prefix));
        //set engine
        header.replace("$TbEngine$", createTableEngine(tb.engine));
        //set fields
        header.replace("$Fields$", createFields());
        header.replace("$FieldsWithoutAuto$", createFieldsWithoutAutoIncrement());
        //set database type
        header.replace("$FieldType$", createDatabaseType());
        //set primary keys
        header.replace("$PrimaryKey$", createPrimaryKeys());
        //set index
        header.replace("$FieldIndex$", createIndexFields());
        header.replace("$UniqueFieldIndex$", createIndexFields("unique index"));
        //set check name autoincrement
        header.replace("$CheckNameIncrement$", createCheckNameIncrement());
        //set bind id
        header.replace("$BindAutoIncrementId$", createBindAutoIncrementId());
        //set bind value
        header.replace("$BindValues$", createBindValue());
        //set json to entity
        header.replace("$JsonToEntity$", createJsonToEntity());
        //set entity to json
        header.replace("$EntityToJson$", createEntityToJson());
        //set values getter
        header.replace("$ValuesWithAuto$", createValuesGetWithoutAutoIncrement());
        //set value getter
        header.replace("$GetValueByName$", createGetValueByName());
        //setter and getter
        header.replace("$MemberGetterSetter$", createSetterGetter());
        //set meta type
        header.replace("$DECLARE_META_TYPE$", createMetaType());

        writeContentWithCheckHash(header, getOutputFilePath(tb));
    }
    generateEntityDelegate(tbnames);
    generateConfigureFile(tbnames);
}

QString MysqlGenerator::getFieldCppType(const QString& fieldType) {
    if (fieldType == "tinyint") {
        return "char";
    }
    if (fieldType == "smallint") {
        return "short";
    }
    if (fieldType == "mediumint" || fieldType == "int") {
        return "int";
    }
    if (fieldType == "bigint") {
        return "qint64";
    }
    if (fieldType == "float" || fieldType == "double" || fieldType == "decimal") {
        return "qreal";
    }

    if (fieldType == "date") {
        return "QDate";
    }
    if (fieldType == "time") {
        return "QString"; //see qsql_mysql.cpp#313
    }
    if (fieldType == "datetime" || fieldType == "timestamp") {
        return "QDateTime";
    }

    if (fieldType == "char") {
        return "QChar";
    }
    if (fieldType == "varchar" || fieldType == "tinytext" || fieldType == "text" ||
        fieldType == "mediumtext" || fieldType == "longtext") {
        return "QString";
    }
    if (fieldType == "tinyblob" || fieldType == "blob" || fieldType == "mediumblob" ||
        fieldType == "longblob") {
        return "QByteArray";
    }

    return QString("unknown");
}

QString MysqlGenerator::getCppDefaultValueString(const QString& fieldType, const QString& defaultValue) {
    if (fieldType == "tinyint") {
        if (defaultValue.toLower() == "null") {
            return "char()";
        }
        return defaultValue;
    }
    if (fieldType == "smallint") {
        if (defaultValue.toLower() == "null") {
            return "short()";
        }
        return defaultValue;
    }
    if (fieldType == "int" || fieldType == "mediumint") {
        if (defaultValue.toLower() == "null") {
            return "int()";
        }
        return defaultValue;
    }
    if (fieldType == "bigint") {
        if (defaultValue.toLower() == "null") {
            return "qint64()";
        }
        return defaultValue;
    }
    if (fieldType == "float" || fieldType == "double" || fieldType == "decimal") {
        if (defaultValue.toLower() == "null") {
            return "qreal()";
        }
        return defaultValue;
    }
    if (fieldType == "time") {
        if (defaultValue.toLower() == "now") {
            return "QTime::currentTime().toString(\"HH:mm:ss\")";
        }
        if (defaultValue.toLower() == "null") {
            return "QString()";
        }
        if (defaultValue.contains("QTime")) {
            return defaultValue;
        }
        return QString("\"%1\"").arg(defaultValue);
    }
    if (fieldType == "date") {
        if (defaultValue.toLower() == "now") {
            return "QDate::currentDate()";
        }
        if (defaultValue.toLower() == "null") {
            return "QDate()";
        }
        if (defaultValue.contains("QDate")) {
            return defaultValue;
        }
        return QString("QDate::fromString(\"%1\")").arg(defaultValue);
    }
    if (fieldType == "datetime" || fieldType == "timestamp") {
        if (defaultValue.toLower() == "now") {
            return "QDateTime::currentDateTime()";
        }
        if (defaultValue.toLower() == "null") {
            return "QDateTime()";
        }
        if (defaultValue.contains("QDateTime")) {
            return defaultValue;
        }
        if (defaultValue.startsWith("NULL ON UPDATE CURRENT_TIMESTAMP")) {
            return "QDateTime()";
        }
        if (defaultValue.startsWith("CURRENT_TIMESTAMP")) {
            return "QDateTime::currentDateTime()";
        }
        return QString("QDateTime::fromString(\"%1\")").arg(defaultValue);
    }
    if (fieldType == "char") {
        if (defaultValue.toLower() == "null") {
            return "QChar()";
        }
        return QString("'%1'").arg(defaultValue);
    }
    if (fieldType == "varchar" || fieldType == "tinytext" || fieldType == "text" ||
        fieldType == "mediumtext" || fieldType == "longtext") {
        if (defaultValue.toLower() == "null" || defaultValue.toLower() == "empty") {
            return "QString()";
        }
        if (defaultValue.contains("QString")) {
            return defaultValue;
        }
        if (defaultValue.startsWith('"')) {
            return defaultValue;
        }
        return QString("\"%1\"").arg(defaultValue);
    }

    if (fieldType == "tinyblob" || fieldType == "blob" || fieldType == "mediumblob" ||
        fieldType == "longblob") {
        if (defaultValue.toLower() == "null" || defaultValue.toLower() == "empty") {
            return "QByteArray()";
        }
        if (defaultValue.contains("QByteArray")) {
            return defaultValue;
        }
        if (defaultValue.startsWith('"')) {
            return defaultValue;
        }
        return QString("\"%1\"").arg(defaultValue);
    }
    return "unknown";
}

QString MysqlGenerator::getDatabaseDefaultValueString(const QString& fieldType, const QString& defaultValue) {
    if (fieldType == "tinyint" || fieldType == "smallint" || fieldType == "mediumint" || fieldType == "int" ||
        fieldType == "bigint" || fieldType == "float" || fieldType == "double" || fieldType == "decimal") {
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        return defaultValue;
    }
    if (fieldType == "time" || fieldType == "date") {
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        if (defaultValue.toLower() == "now") {
            return "null";
        }
        if (defaultValue.contains("QTime") || defaultValue.contains("QDate")) {
            return "null";
        }
        return QString("'%1'").arg(defaultValue);
    }
    if (fieldType == "datetime" || fieldType == "timestamp") {
        if (defaultValue.toLower() == "now") {
            return "CURRENT_TIMESTAMP";
        }
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        if (defaultValue.contains("QDateTime")) {
            return "null";
        }
        if (defaultValue.contains("CURRENT_TIMESTAMP")) {
            return defaultValue;
        }
        return QString("'%1'").arg(defaultValue);
    }
    if (fieldType == "char") {
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        return QString("'%1'").arg(defaultValue);
    }
    if (fieldType == "varchar") {
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        if (defaultValue.toLower() == "empty") {
            return "''";
        }
        if (defaultValue.contains("QString")) {
            return "null";
        }
        if (defaultValue.startsWith('"')) {
            return QString(defaultValue).replace('"', '\'');
        }
        return QString("'%1'").arg(defaultValue);
    }
    return "null";
}

QString MysqlGenerator::getDatabaseFieldType(const QString& fieldType) {
    return fieldType;
}

QString MysqlGenerator::getComment(const QString& note) {
    return QString(" comment '%1'").arg(note);
}

QString MysqlGenerator::getAutoIncrementStatement() {
    return QString("auto_increment");
}

QString MysqlGenerator::getSqlTypeName() {
    return "Mysql";
}

QString MysqlGenerator::getSqlClientTypeName() {
    return "ClientMysql";
}
