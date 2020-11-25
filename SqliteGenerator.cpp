#include "SqliteGenerator.h"

SqliteGenerator::SqliteGenerator(QString outputPath, SqliteEntity entity, QString dbloadPath)
    : AbstractGenerator(outputPath, dbloadPath)
    , outputPath(outputPath)
    , entity(entity)
{
}

void SqliteGenerator::generate() {
    QString hppTemplate = loadTemplateFile(":/generator/templates/sqlite.txt");
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
        header.replace("$FiedIdInit$", createDefaultConstruct());
        header.replace("$ConstructFields$", createConstructField());
        header.replace("$ConstructCommit$", createConstructCommit());
        //set field declare
        header.replace("$FieldDeclare$", createFieldDeclare());
        //set field size
        header.replace("$FieldSize$", createFieldSize());
        //set tablename
        header.replace("$TbName$", createTableName(entity.prefix));
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
        //setter and getter
        header.replace("$MemberGetterSetter$", createSetterGetter());
        //set meta type
        header.replace("$DECLARE_META_TYPE$", createMetaType());

        writeTableHeaderByDiff(header, tb);
    }
    generateEntityDelegate(tbnames);
}

QString SqliteGenerator::getFieldCppType(const QString& fieldType) {
    if (fieldType == "int") {
        return "int";
    }
    if (fieldType == "long") {
        return "qint64";
    }
    if (fieldType == "real") {
        return "qreal";
    }
    if (fieldType == "text") {
        return "QString";
    }
    if (fieldType == "blob") {
        return "QByteArray";
    }
    if (fieldType == "variant") {
        return "QVariant";
    }
    return QString("unknown");
}

bool SqliteGenerator::checkFieldStrType(const QString& fieldType) {
    if (fieldType == "text") {
        return true;
    }
    return false;
}

bool SqliteGenerator::checkFieldDecimalType(const QString& fieldType) {
    if (fieldType == "int") {
        return true;
    }
    if (fieldType == "long") {
        return true;
    }
    if (fieldType == "real") {
        return true;
    }
    return false;
}

QString SqliteGenerator::getDatabaseFieldType(const QString& fieldType) {
    if (fieldType == "int") {
        return "integer";
    }
    if (fieldType == "long") {
        return "integer";
    }
    if (fieldType == "variant") {
        return "blob";
    }
    return fieldType;
}

QString SqliteGenerator::getComment(const QString& note) {
    return QString();
}

QString SqliteGenerator::getAutoIncrementStatement() {
    return QString("autoincrement");
}

QString SqliteGenerator::getSqlNamespaceName() {
    return "DaoSqlite";
}
