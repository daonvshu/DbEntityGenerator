#include "SqlServerGenerator.h"

SqlServerGenerator::SqlServerGenerator(QString outputPath, Entity entity, QString dbloadPath)
    : AbstractGenerator(outputPath, dbloadPath)
    , entity(entity) {
}

void SqlServerGenerator::generate() {
    QString hppTemplate = loadTemplateFile(":/generator/templates/sqlserver.txt");
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
        header.replace("$FieldsWithoutTimestamp$", createFieldsWithoutTimestamp());
        //set database type
        header.replace("$FieldType$", createDatabaseType());
        //set primary keys
        header.replace("$PrimaryKey$", createPrimaryKeys());
        //set index
        header.replace("$ClusteredFieldIndex$", createIndexFields("clustered"));
        header.replace("$UniqueClusteredFieldIndex$", createIndexFields("unique clustered"));
        header.replace("$NonClusteredFieldIndex$", createIndexFields("nonclustered"));
        header.replace("$UniqueNonClusteredFieldIndex$", createIndexFields("unique nonclustered"));
        header.replace("$GetIndexOption$", createIndexOption());
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

QString SqlServerGenerator::getFieldCppType(const QString& fieldType) {
    if (fieldType == "tinyint") {
        return "uchar";
    }
    if (fieldType == "smallint") {
        return "short";
    }
    if (fieldType == "int") {
        return "int";
    }
    if (fieldType == "bigint") {
        return "qint64";
    }
    if (fieldType == "float" || fieldType == "double" || fieldType == "decimal" ||
        fieldType == "numeric" || fieldType == "real") {
        return "qreal";
    }

    if (fieldType == "date") {
        return "QDate";
    }
    if (fieldType == "time") {
        return "QTime";
    }
    if (fieldType == "datetime" || fieldType == "datetime2" || fieldType == "datetimeoffset") {
        return "QDateTime";
    }
    if (fieldType == "timestamp") {
        return "QByteArray";
    }

    if (fieldType == "char" || fieldType == "varchar" || fieldType == "varchar(max)" || 
        fieldType == "nchar" || fieldType == "nvarchar" || fieldType == "nvarchar(max)" ||
        fieldType == "text" || fieldType == "ntext") {
        return "QString";
    }
    if (fieldType == "bit") {
        return "bool";
    }
    if (fieldType == "binary" || fieldType == "varbinary" || fieldType == "varbinary(max)") {
        return "QByteArray";
    }

    if (fieldType == "sql_variant") {
        return "QVariant";
    }
    if (fieldType == "uniqueidentifier" || fieldType == "xml") {
        return "QByteArray";
    }

    return QString("unknown");
}

QString SqlServerGenerator::getCppDefaultValueString(const QString& fieldType, const QString& defaultValue) {
    if (fieldType == "tinyint") {
        if (defaultValue.toLower() == "null") {
            return "uchar()";
        }
        return defaultValue;
    }
    if (fieldType == "smallint") {
        if (defaultValue.toLower() == "null") {
            return "short()";
        }
        return defaultValue;
    }
    if (fieldType == "int") {
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
    if (fieldType == "float" || fieldType == "double" || fieldType == "decimal" || fieldType == "numeric" || fieldType == "real") {
        if (defaultValue.toLower() == "null") {
            return "qreal()";
        }
        return defaultValue;
    }
    if (fieldType == "time") {
        if (defaultValue.toLower() == "now") {
            return "QTime::currentTime()";
        }
        if (defaultValue.toLower() == "null") {
            return "QTime()";
        }
        if (defaultValue.contains("QTime")) {
            return defaultValue;
        }
        return QString("QTime::fromString(\"%1\")").arg(defaultValue);
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
    if (fieldType == "datetime" || fieldType == "datetime2" || fieldType == "datetimeoffset") {
        if (defaultValue.toLower() == "now") {
            return "QDateTime::currentDateTime()";
        }
        if (defaultValue.toLower() == "null") {
            return "QDateTime()";
        }
        if (defaultValue.contains("QDateTime")) {
            return defaultValue;
        }
        return QString("QDateTime::fromString(\"%1\")").arg(defaultValue);
    }
    if (fieldType == "timestamp") {
        return "QByteArray()";
    }
    if (fieldType == "char" || fieldType == "varchar" || fieldType == "varchar(max)" ||
        fieldType == "nchar" || fieldType == "nvarchar" || fieldType == "nvarchar(max)" ||
        fieldType == "text" || fieldType == "ntext") {
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
    if (fieldType == "bit") {
        if (defaultValue.toLower() == "null") {
            return "bool()";
        }
        return defaultValue;
    }
    if (fieldType == "binary" || fieldType == "varbinary" || fieldType == "varbinary(max)") {
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
    if (fieldType == "sql_variant") {
        return defaultValue;
    }
    if (fieldType == "uniqueidentifier" || fieldType == "xml") {
        return defaultValue;
    }
    return "unknown";
}

QString SqlServerGenerator::getDatabaseDefaultValueString(const QString& fieldType, const QString& defaultValue) {
    if (fieldType == "tinyint" || fieldType == "smallint" || fieldType == "int" ||
        fieldType == "bigint" || fieldType == "float" || fieldType == "double" || fieldType == "decimal" || fieldType == "numeric" ||
        fieldType == "bit") {
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        return defaultValue;
    }
    if (fieldType == "date" || fieldType == "time" || fieldType == "datetime" || fieldType == "datetime2" || fieldType == "datetimeoffset") {
        if (defaultValue.toLower() == "null") {
            return "null";
        }
        if (defaultValue.toLower() == "now") {
            return "getdate()";
        }
        if (defaultValue.contains("QTime") || defaultValue.contains("QDate") || defaultValue.contains("QDateTime")) {
            return "null";
        }
        return QString("'%1'").arg(defaultValue);
    }
    if (fieldType == "timestamp") {
        return "null";
    }
    if (fieldType == "char" || fieldType == "varchar" || fieldType == "varchar(max)" ||
        fieldType == "nchar" || fieldType == "nvarchar" || fieldType == "nvarchar(max)" ||
        fieldType == "text" || fieldType == "ntext") {
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
    if (fieldType == "binary" || fieldType == "varbinary" || fieldType == "varbinary(max)") {
        return "null";
    }
    if (fieldType == "sql_variant" || fieldType == "uniqueidentifier" || fieldType == "xml") {
        return "null";
    }
    return "null";
}

QString SqlServerGenerator::getDatabaseFieldType(const QString& fieldType) {
    return fieldType;
}

QString SqlServerGenerator::getComment(const QString& note) {
    Q_UNUSED(note);
    return QString();
}

QString SqlServerGenerator::getAutoIncrementStatement() {
    return QString("identity(1,1)"); //from 1 step 1
}

QString SqlServerGenerator::getSqlTypeName() {
    return "SqlServer";
}

QString SqlServerGenerator::getSqlClientTypeName() {
    return "ClientSqlServer";
}
