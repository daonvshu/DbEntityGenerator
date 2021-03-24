#include "AbstractGenerator.h"

#include <qcryptographichash.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qset.h>

AbstractGenerator::AbstractGenerator(QString outputPath, QString loadPath)
    : outputPath(outputPath)
    , dbloadPath(loadPath)
{
    if (!dbloadPath.isEmpty() && !dbloadPath.endsWith("/")) {
        dbloadPath += "/";
    }
}

QString AbstractGenerator::getOutputFilePath(const Table& table) {
    return getOutputFilePath(table.name + ".h");
}

QString AbstractGenerator::getOutputFilePath(const QString& fileName) {
    return QString("%1/%2").arg(outputPath).arg(fileName);
}

QByteArray AbstractGenerator::getFileMd5(const QString& filePath) {
    QFile file(filePath);
    if (!file.exists()) {
        return QByteArray();
    }
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        data = QString(data).toUtf8();
        if (!data.isEmpty()) {
            return QCryptographicHash::hash(data, QCryptographicHash::Md5);
        }
    }
    return QByteArray();
}

QString AbstractGenerator::loadTemplateFile(const QString& name) {
    QFile file(name);
    file.open(QIODevice::ReadOnly);
    auto data = file.readAll();
    file.close();
    return data;
}

QString AbstractGenerator::lowerAndSplitWithUnderline(const QString& s) {
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

QString AbstractGenerator::upperFirstChar(const QString& s) {
    auto p1data = s.toLatin1();
    char p1char = p1data.at(0);
    return (char)(p1char - 32) + p1data.mid(1);
}

QString AbstractGenerator::getFieldInDatabaseName(const QString& s, bool ignoreMix) {
    return lowerAndSplitWithUnderline(s);
}

void AbstractGenerator::writeTableHeaderByDiff(const QString& content, const Table& tb) {
    auto hppCotent = QString(content).toUtf8();
    auto currentHash = QCryptographicHash::hash(hppCotent, QCryptographicHash::Md5);
    QString outputFile = getOutputFilePath(tb);
    auto oldHash = getFileMd5(outputFile);
    if (oldHash.compare(currentHash) != 0) {
        writeUtf8ContentWithBomHeader(outputFile, content);
    }
}

#define TP_START            QString str
#define ADD(STR)            str.append(STR)
#define ADD_S(STR)          ADD('"' + STR + '"')
#define ADD_s(STR)          ADD('\'' + STR + '\'')
#define TAB_1               ADD("    ")
#define TAB_2               ADD("        ")
#define TAB_4               ADD("                ")
#define ENTER               ADD("\n")
#define SPACE               ADD(" ")
#define COMMENT_2           ADD("//")
#define COMMENT_3           ADD("///")
#define SEMICOLON           ADD(";")
#define COMMA               ADD(",")
#define EQUAL               ADD(" = ")
#define TP_END              return str

#define FIELD_FOREACH(fields)    for (const auto& field : fields)
#define INDEX_FOREACH(indexes)    for (const auto& index : indexes)
#define DECLARE_FIELD   \
ADD(getFieldCppType(field.type));\
SPACE;\
ADD(field.name)

#define DECLARE_CONST_FIELD   \
ADD("const " + getFieldCppType(field.type) + "&");\
SPACE;\
ADD(field.name)

#define USE_LEFT(size)    str.chop(size)
#define USE_RIGHT(size) str = str.mid(size)
#define FIELD_INIT(field)   ADD(field + '(' + field + ')')

QString AbstractGenerator::createFieldList() {
    TP_START;
    QList<Field> transientFields;
    currentPrimaryKeySize = 0;
    FIELD_FOREACH(tb.fields) {
        if (!field.transient) {
            TAB_1;
            COMMENT_2;
            ADD(field.note);
        } else {
            transientFields << field;
            continue;
        }
        if (field.constraint == "primary key") {
            currentPrimaryKeySize++;
        }
        ENTER;
        TAB_1;
        DECLARE_FIELD;
        SEMICOLON;
        ENTER;
    }
    if (!transientFields.isEmpty()) {
        ENTER;
        FIELD_FOREACH(transientFields) {
            TAB_1;
            COMMENT_3;
            ADD("transient");
            SPACE;
            ADD(field.note);
            ENTER;
            TAB_1;
            DECLARE_FIELD;
            SEMICOLON;
            ENTER;
        }
    }
    currentFieldSize = tb.fields.size() - transientFields.size();

    TP_END;
}
/*
$ClassName$() {
    $FieldIdInit$
}

$ClassName$(
    $ConstructFields$
) : $ConstructCommit$
{ }

$ClassName$(
    $ConstructFieldsWithoutDefault$
) : $ConstructCommitWithoutDefault$
{ }
*/
QString AbstractGenerator::createConstruct() {
    TP_START;
    QStringList constructList; //hold the append sequence
    QSet<QString> constructSet;
    //default construct
    auto fieldInit = createDefaultFieldInit();
    if (!fieldInit.isEmpty()) {
        auto str = QString("\n\n    $ClassName$() {\n%1    }").arg(fieldInit);
        constructList << str;
        constructSet << str;
    } else {
        auto str = "\n\n    $ClassName$() {\n    }";
        constructList << str;
        constructSet << str;
    }

    bool hasNotDefaultValue = false;
    bool hasNotAutoIncrementField = false;
    FIELD_FOREACH(tb.fields) {
        if (field.default.isEmpty() && !field.autoincreament) {
            hasNotDefaultValue = true;
        }
        if (!field.autoincreament) {
            hasNotAutoIncrementField = true;
        }
    }

    if (hasNotAutoIncrementField) {
        auto str = QString("\n\n    $ClassName$(\n%1\n    ) : %2\n    {\n%3    }")
            .arg(createConstructField(), createConstructCommit(), createDefaultFieldInit(true));
        if (!constructSet.contains(str)) {
            constructSet << str;
            constructList << str;
        }
    }

    if (hasNotDefaultValue) {
        auto str = QString("\n\n    $ClassName$(\n%1\n    ) : %2\n    {\n%3    }")
            .arg(createConstructField(true), createConstructCommit(true), fieldInit);
        if (!constructSet.contains(str)) {
            constructSet << str;
            constructList << str;
        }
    }

    for (const auto& customConstructor : tb.customConstructor) {
        if (!customConstructor.isEmpty()) {
            auto str = QString("\n\n    $ClassName$(\n%1\n    ) : %2\n    {\n%3    }")
                .arg(
                    createConstructField(true, customConstructor),
                    createConstructCommit(true, customConstructor),
                    createDefaultFieldInit(false, customConstructor)
                );
            if (!constructSet.contains(str)) {
                constructSet << str;
                constructList << str;
            }
        }
    }

    for (const auto& construct : constructList) {
        ADD(construct);
    }

    str.replace("$ClassName$", tb.name);

    TP_END;
}

QString AbstractGenerator::createDefaultFieldInit(bool onlyAutoIncrement, const QStringList& excludeFieldsWithDefault) {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.default.isEmpty()) {
            continue;
        }
        if (onlyAutoIncrement && !field.autoincreament) {
            continue;
        }
        if (!excludeFieldsWithDefault.isEmpty()) {
            if (excludeFieldsWithDefault.contains(field.name)) {
                continue;
            }
        }
        TAB_2;
        ADD(field.name);
        EQUAL;
        ADD(getCppDefaultValueString(field.type, field.default));
        SEMICOLON;
        ENTER;
    }
    TP_END;
}

QString AbstractGenerator::createConstructField(bool skipDefaultValue, const QStringList& enforceFields) {
    TP_START;
    if (tb.fields.isEmpty()) {
        TP_END;
    }
    FIELD_FOREACH(tb.fields) {
        if (enforceFields.isEmpty()) {
            if (field.transient) {
                continue;
            }
            if (field.autoincreament) {
                continue;
            }
            if (!field.default.isEmpty() && skipDefaultValue) {
                continue;
            }
        } else {
            if (!enforceFields.contains(field.name)) {
                continue;
            }
        }
        TAB_2;
        DECLARE_CONST_FIELD;
        COMMA;
        ENTER;
    }
    USE_LEFT(2);
    TP_END;
}

QString AbstractGenerator::createConstructCommit(bool skipDefaultValue, const QStringList& enforceFields) {
    TP_START;
    if (tb.fields.isEmpty()) {
        TP_END;
    }
    FIELD_FOREACH(tb.fields) {
        if (enforceFields.isEmpty()) {
            if (field.transient) {
                continue;
            }
            if (field.autoincreament) {
                continue;
            }
            if (!field.default.isEmpty() && skipDefaultValue) {
                continue;
            }
        } else {
            if (!enforceFields.contains(field.name)) {
                continue;
            }
        }
        FIELD_INIT(field.name);
        ENTER;
        TAB_1;
        COMMA;
        SPACE;
    }
    USE_LEFT(7);
    TP_END;
}

QString AbstractGenerator::createFieldDeclare(const QString& prefix) {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ENTER;
        TAB_2;
        ADD(QString("EntityField<%1> %2 = EntityField<%1>(\"%3\", \"%4\");")
            .arg(getFieldCppType(field.type))
            .arg(field.name)
            .arg(getFieldInDatabaseName(field.name))
            .arg(createTableName(prefix))
        );
    }
    TP_END;
}

QString AbstractGenerator::createFieldDeclareReset() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ENTER;
        TAB_1;
        TAB_2;
        ADD(QString("%1 = EntityField<%2>(\"%3\", tbName);")
            .arg(field.name)
            .arg(getFieldCppType(field.type))
            .arg(getFieldInDatabaseName(field.name))
        );
    }
    TP_END;
}

QString AbstractGenerator::createFieldSize() {
    return QString::number(currentFieldSize);
}

QString AbstractGenerator::createTableName(const QString& prefix) {
    return prefix + tb.name.toLower();
}

QString AbstractGenerator::createTableEngine(const QString& engine) {
    if (engine.isEmpty()) {
        return "QString()";
    } else {
        return "\"" + engine + "\"";
    }
}

QString AbstractGenerator::createFields() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ENTER;
        TAB_4;
        ADD("<< \"");
        ADD(getFieldInDatabaseName(field.name));
        ADD("\"");
    }
    TP_END;
}

QString AbstractGenerator::createFieldsWithoutAutoIncrement() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        if (field.autoincreament) {
            continue;
        }
        ENTER;
        TAB_4;
        ADD("<< \"");
        ADD(getFieldInDatabaseName(field.name));
        ADD("\"");
    }
    TP_END;
}

QString AbstractGenerator::createFieldsWithoutTimestamp() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        if (field.type == "timestamp") {
            continue;
        }
        ENTER;
        TAB_4;
        ADD("<< \"");
        ADD(getFieldInDatabaseName(field.name));
        ADD("\"");
    }
    TP_END;
}

QString AbstractGenerator::createDatabaseType() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ENTER;
        TAB_4;
        ADD("<< QStringLiteral(\"");
        ADD(getFieldInDatabaseName(field.name));
        SPACE;
        ADD(getDatabaseFieldType(field.type));
        if (field.bitsize != 0) {
            if (field.type == "decimal" && field.decimal_d != 0) {
                ADD(QString("(%1,%2)").arg(QString::number(field.bitsize), QString::number(field.decimal_d)));
            } else {
                ADD(QString("(%1)").arg(QString::number(field.bitsize)));
            }
        } else {
            if (field.type == "decimal" && field.decimal_d != 0) {
                ADD(QString("(0,%1)").arg(QString::number(field.decimal_d)));
            }
        }
        if (!field.constraint.isEmpty()) {
            if (field.constraint == "primary key" && currentPrimaryKeySize == 1) {
                SPACE;
                ADD(field.constraint);
                if (field.autoincreament) {
                    SPACE;
                    ADD(getAutoIncrementStatement());
                }
            } else if (field.constraint == "not null") {
                SPACE;
                ADD(field.constraint);
            } else if (field.constraint == "unique") {
                SPACE;
                ADD("not null");
                SPACE;
                ADD(field.constraint);
            }
        }
        if (!field.default.isEmpty() && !field.autoincreament) {
            auto defaultStr = getDatabaseDefaultValueString(field.type, field.default);
            if (!defaultStr.isEmpty()) {
                SPACE;
                if (field.constraint != "primary key") {
                    ADD("null ");
                }
                ADD("default ");
                ADD(defaultStr);
            }
        }
        auto comment = getComment(field.note);
        if (!comment.isEmpty()) {
            ADD(getComment(field.note));
        }
        ADD("\")");
    }
    TP_END;
}

QString AbstractGenerator::createPrimaryKeys() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        if (field.constraint == "primary key") {
            ADD(" << ");
            ADD_S(getFieldInDatabaseName(field.name));
        }
    }
    TP_END;
}

QString AbstractGenerator::createIndexFields(QString indexType) {
    TP_START;
    INDEX_FOREACH(tb.indexes) {
        if (index.indexType == indexType) {
            ENTER;
            TAB_4;
            ADD("<< (QStringList()");
            for (const auto& s : index.fields) {
                ADD(" << ");
                ADD_S(getFieldInDatabaseName(s));
            }
            ADD(")");
        }
    }
    TP_END;
}

QString AbstractGenerator::createIndexOption() {
    TP_START;
    INDEX_FOREACH(tb.indexes) {
        if (!index.indexOptions.isEmpty()) {
            ADD("if(name == \"");
            QString indexName = "index";
            for (const auto& field : index.fields) {
                indexName.append("_").append(field.split(" ").at(0));
            }
            ADD(indexName);
            ADD("\") {");
            ENTER;
            TAB_4;
            ADD("return \"");
            ADD(index.indexOptions);
            ADD("\";");
            ENTER;
            TAB_1;
            TAB_2;
            ADD("}");
            ENTER;
            TAB_1;
            TAB_2;
        }
    }
    if (str.isEmpty()) {
        ADD("Q_UNUSED(name);");
        ENTER;
        TAB_1;
        TAB_2;
    }
    TP_END;
}

QString AbstractGenerator::createCheckNameIncrement() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        if (field.autoincreament) {
            if (!str.isEmpty()) {
                ENTER;
                TAB_4;
                TAB_2;
                ADD("|| ");
            }
            ADD(QString("name == \"%1\"").arg(getFieldInDatabaseName(field.name)));
        }
    }
    if (str.isEmpty()) {
        ADD("Q_UNUSED(name);");
        ENTER;
        TAB_1;
        TAB_2;
        ADD("return false");
    } else {
        str.prepend("return ");
    }
    TP_END;
}

QString AbstractGenerator::createValuesGetWithoutAutoIncrement() {
    TP_START;
    bool set = false;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        if (field.autoincreament) {
            continue;
        }
        ENTER;
        TAB_4;
        ADD("<< entity.");
        ADD(field.name);
        set = true;
    }
    if (set) {
        str.prepend("return QVariantList()");
    } else {
        ADD("Q_UNUSED(entity);");
        ENTER;
        TAB_1;
        TAB_2;
        ADD("return QVariantList()");
    }
    TP_END;
}

QString AbstractGenerator::createGetValueByName() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ADD("if (target == \"");
        ADD(getFieldInDatabaseName(field.name));
        ADD("\") {");
        ENTER;
        TAB_4;
        ADD("return entity.");
        ADD(field.name);
        ADD(";");
        ENTER;
        TAB_1;
        TAB_2;
        ADD("}");
        ENTER;
        TAB_1;
        TAB_2;
    }
    ADD("return entity.__extra.value(target);");
    TP_END;
}

QString AbstractGenerator::createBindAutoIncrementId() {
    TP_START;
    bool set = false;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        if (field.autoincreament) {
            ADD("entity.");
            ADD(field.name);
            ADD(" = id.value<");
            ADD(getFieldCppType(field.type));
            ADD(">();");
            set = true;
            break;
        }
    }
    if (!set) {
        ADD("Q_UNUSED(entity);");
        ENTER;
        TAB_1;
        TAB_2;
        ADD("Q_UNUSED(id);");
    }
    TP_END;
}

QString AbstractGenerator::createBindValue() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ADD(" else if (target == \"");
        ADD(getFieldInDatabaseName(field.name));
        ADD("\") {");
        ENTER;
        TAB_4;
        ADD("entity.");
        ADD(field.name);
        ADD(" = value.value<");
        ADD(getFieldCppType(field.type));
        ADD(">();");
        ENTER;
        TAB_1;
        TAB_2;
        ADD("}");
    }
    if (!str.isEmpty()) {
        ADD(" else {");
        ENTER;
        TAB_4;
        ADD("entity.__putExtra(target, value);");
        ENTER;
        TAB_1;
        TAB_2;
        ADD("}");
    }
    USE_RIGHT(6);
    TP_END;
}

QString AbstractGenerator::createJsonToEntity() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ENTER;
        TAB_1;
        TAB_2;
        ADD("entity.");
        ADD(field.name);
        ADD(" = ");
        auto cppType = getFieldCppType(field.type);
        if (cppType == "QByteArray") {
            ADD("QByteArray::fromBase64(object.value(\"");
        } else if (cppType == "QDate") {
            ADD("QDate::fromString(object.value(\"");
        } else if (cppType == "QDateTime") {
            ADD("QDateTime::fromString(object.value(\"");
        } else if (cppType == "QTime") {
            ADD("QTime::fromString(object.value(\"");
        } else {
            ADD("object.value(\"");
        }

        if (field.jsonKey.isEmpty()) {
            ADD(getFieldInDatabaseName(field.name, true));
        } else {
            ADD(field.jsonKey);
        }

        if (cppType == "QByteArray") {
            ADD("\").toString().toLatin1());");
        } else if (cppType == "QDate" || cppType == "QTime" || cppType == "QDateTime") {
            ADD("\").toString(), \"");
            if (field.jsonTimeFormat.isEmpty()) {
                if (cppType == "QDate") {
                    ADD("yyyy-MM-dd");
                } else if (cppType == "QTime") {
                    ADD("HH:mm:ss");
                } else {
                    ADD("yyyy-MM-dd HH:mm:ss");
                }
            } else {
                ADD(field.jsonTimeFormat);
            }
            ADD("\");");
        } else if (cppType == "QVariant") {
            ADD("\");");
        } else {
            ADD("\").toVariant().value<");
            ADD(getFieldCppType(field.type));
            ADD(">();");
        }
    }
    TP_END;
}

QString AbstractGenerator::createEntityToJson() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ENTER;
        TAB_1;
        TAB_2;
        ADD("object.insert(\"");
        if (field.jsonKey.isEmpty()) {
            ADD(getFieldInDatabaseName(field.name, true));
        } else {
            ADD(field.jsonKey);
        }
        ADD("\", ");
        auto cppType = getFieldCppType(field.type);
        if (cppType == "QByteArray") {
            ADD("QString::fromLatin1(entity.");
            ADD(field.name);
            ADD(".toBase64())");
        } else if (cppType == "QVariant") {
            ADD("QJsonValue::fromVariant(entity.");
            ADD(field.name);
            ADD(")");
        } else {
            ADD("entity.");
            ADD(field.name);
            if (cppType == "QDate" || cppType == "QTime" || cppType == "QDateTime") {
                ADD(".toString(\"");
                if (field.jsonTimeFormat.isEmpty()) {
                    if (cppType == "QDate") {
                        ADD("yyyy-MM-dd");
                    } else if (cppType == "QTime") {
                        ADD("HH:mm:ss");
                    } else {
                        ADD("yyyy-MM-dd HH:mm:ss");
                    }
                } else {
                    ADD(field.jsonTimeFormat);
                }
                ADD("\")");
            } else if (cppType == "QChar") {
                ADD(".toLatin1()");
            }
        }
        ADD(");");
    }
    TP_END;
}

QString AbstractGenerator::createSetterGetter() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        //setter
        ENTER;
        TAB_1;
        COMMENT_2;
        if (!field.note.isEmpty()) {
            ADD("set " + field.note);
        }
        ENTER;
        TAB_1;
        ADD("inline void set");
        ADD(upperFirstChar(field.name));
        ADD("(");
        DECLARE_CONST_FIELD;
        
        ADD(") {this->");
        ADD(field.name);
        EQUAL;
        ADD(field.name);
        ADD(";}");
        //getter
        ENTER;
        TAB_1;
        COMMENT_2;
        if (!field.note.isEmpty()) {
            ADD("get " + field.note);
        }
        ENTER;
        TAB_1;
        ADD("inline ");
        ADD(getFieldCppType(field.type));
        ADD(" get");
        ADD(upperFirstChar(field.name));
        ADD("() const {return ");
        ADD(field.name);
        ADD(";}");
    }
    TP_END;
}

QString AbstractGenerator::createMetaType() {
    if (tb.metatype) {
        return QString("Q_DECLARE_METATYPE(%1);").arg(tb.name);
    }
    return QString();
}

void AbstractGenerator::generateEntityDelegate(QStringList tbNames) {
    auto header = loadTemplateFile(":/generator/templates/entitydelegate_h.txt");
    header.replace("$SqlType$", getSqlTypeName());
    QString headerOutputFile = getOutputFilePath(QString("%1EntityInclude.h").arg(getSqlTypeName()));
    writeUtf8ContentWithBomHeader(headerOutputFile, header);

    auto cpp = loadTemplateFile(":/generator/templates/entitydelegate_cpp.txt");
    cpp.replace("$SqlType$", getSqlTypeName());
    cpp.replace("$SqlClientType$", getSqlClientTypeName());

    QString entityHeaders;
    QString entityListStr;
    for (const auto& name : tbNames) {
        entityHeaders.append(QString("#include \"%1.h\"\n").arg(name));
        entityListStr.append(name).append(", ");
    }
    cpp.replace("$EntityHeaders$", entityHeaders);
    cpp.replace("$EntityList$", entityListStr.chopped(2));
    cpp.replace("$DbLoaderPath$", dbloadPath);

    QString cppOutputFile = getOutputFilePath(QString("%1EntityInclude.cpp").arg(getSqlTypeName()));
    writeUtf8ContentWithBomHeader(cppOutputFile, cpp);
}

void AbstractGenerator::writeUtf8ContentWithBomHeader(const QString& path, const QString& content) {
    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream stream(&file);
    stream.setCodec("utf-8");
    stream.setGenerateByteOrderMark(true);
    stream << content;
    file.flush();
    file.close();
}
