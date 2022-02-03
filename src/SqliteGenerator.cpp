#include "SqliteGenerator.h"

SqliteGenerator::SqliteGenerator(QString outputPath, Entity entity, QString dbloadPath)
    : AbstractGenerator(outputPath, dbloadPath)
    , entity(entity){
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
        header.replace("$Construct$", createConstruct());
        //set field declare
        header.replace("$FieldDeclare$", createFieldDeclare(entity.prefix));
        header.replace("$FieldDeclareReset$", createFieldDeclareReset());
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

QString SqliteGenerator::getCppDefaultValueString(const QString& fieldType, const QString& defaultValue) {
    if (fieldType == "int") {
        if (defaultValue.toLower() == "null") {
            return "int()";
        }
        return defaultValue;
    }
    if (fieldType == "long") {
        if (defaultValue.toLower() == "null") {
            return "qint64()";
        }
        return defaultValue;
    }
    if (fieldType == "real") {
        if (defaultValue.toLower() == "null") {
            return "qreal()";
        }
        return defaultValue;
    }
    if (fieldType == "text") {
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
    if (fieldType == "blob") {
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
    if (fieldType == "variant") {
        return defaultValue;
    }
    return "unknown";
}

QString SqliteGenerator::getDatabaseDefaultValueString(const QString& fieldType, const QString& defaultValue) {
    if (fieldType == "int" || fieldType == "long" || fieldType == "real") {
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        return defaultValue;
    }
    if (fieldType == "text" || fieldType == "blob") {
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        if (defaultValue.toLower() == "empty") {
            return "''";
        }
        if (fieldType == "text" && defaultValue.contains("QString")) {
            return "null";
        }
        if (fieldType == "blob" && defaultValue.contains("QByteArray")) {
            return "null";
        }
        if (defaultValue.startsWith('"')) {
            return QString(defaultValue).replace('"', '\'');
        }
        return QString("'%1'").arg(defaultValue);
    }
    return "null";
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
    Q_UNUSED(note);
    return QString();
}

QString SqliteGenerator::getAutoIncrementStatement() {
    return QString("autoincrement");
}

QString SqliteGenerator::getSqlTypeName() {
    return "Sqlite";
}

QString SqliteGenerator::getSqlClientTypeName() {
    return "ClientSqlite";
}
