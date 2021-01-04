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
        //set values getter
        header.replace("$ValuesWithAuto$", createValuesGetWithoutAutoIncrement());
        //set value getter
        header.replace("$GetValueByName$", createGetValueByName());
        //setter and getter
        header.replace("$MemberGetterSetter$", createSetterGetter());
        //set meta type
        header.replace("$DECLARE_META_TYPE$", createMetaType());

        writeTableHeaderByDiff(header, tb);
    }
    generateEntityDelegate(tbnames);
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
        return "double";
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

    if (fieldType == "variant") {
        return "QVariant";
    }

    return QString("unknown");
}

QString MysqlGenerator::getCppDefaultValueString(const QString& fieldType, const QString& defaultValue) {
    if (fieldType == "tinyint" || fieldType == "smallint" || fieldType == "mediumint" || fieldType == "int" ||
        fieldType == "bigint" || fieldType == "float" || fieldType == "double" || fieldType == "decimal") {
        return defaultValue;
    }
    if (fieldType == "date" || fieldType == "datetime" || fieldType == "timestamp") {
        return defaultValue;
    }
    if (fieldType == "char") {
        return QString("'%1'").arg(defaultValue);
    }
    return QString("\"%1\"").arg(defaultValue);
}

QString MysqlGenerator::getDatabaseDefaultValueString(const QString& fieldType, const QString& defaultValue) {
    if (fieldType == "tinyint" || fieldType == "smallint" || fieldType == "mediumint" || fieldType == "int" ||
        fieldType == "bigint" || fieldType == "float" || fieldType == "double" || fieldType == "decimal") {
        return defaultValue;
    }
    if (fieldType == "date" || fieldType == "datetime" || fieldType == "timestamp") {
        return QString();
    }
    if (fieldType == "char" || fieldType == "varchar") {
        return QString("'%1'").arg(defaultValue);
    }
    return QString();
}

QString MysqlGenerator::getDatabaseFieldType(const QString& fieldType) {
    if (fieldType == "variant") {
        return "blob";
    }
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
