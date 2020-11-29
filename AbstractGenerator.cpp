#include "AbstractGenerator.h"

#include <qcryptographichash.h>
#include <qfile.h>
#include <qregexp.h>

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

        data = QString(data).replace(QRegExp("\\s"), "").toUtf8();
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

void AbstractGenerator::writeTableHeaderByDiff(const QString& content, const Table& tb) {
    auto hppCotent = QString(content).replace(QRegExp("\\s"), "").toUtf8();
    auto currentHash = QCryptographicHash::hash(hppCotent, QCryptographicHash::Md5);
    QString outputFile = getOutputFilePath(tb);
    auto oldHash = getFileMd5(outputFile);
    if (oldHash.compare(currentHash) != 0) {
        QFile file(outputFile);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.write(content.toLocal8Bit());
        file.close();
    }
}

#define TP_START       QString str
#define ADD(STR)        str.append(STR)
#define ADD_S(STR)   ADD('"' + STR + '"')
#define ADD_s(STR)   ADD('\'' + STR + '\'')
#define TAB_1              ADD("    ")
#define TAB_2             ADD("        ")
#define TAB_4             ADD("                ")
#define ENTER             ADD("\n")
#define SPACE              ADD(" ")
#define COMMENT_2   ADD("//")
#define COMMENT_3   ADD("///")
#define SEMICOLON   ADD(";")
#define COMMA            ADD(",")
#define EQUAL             ADD(" = ")
#define TP_END           return str

#define FIELD_FOREACH(fields)    for (const auto& field : fields)
#define INDEX_FOREACH(indexes)    for (const auto& index : indexes)
#define DECLARE_FIELD   \
ADD(getFieldCppType(field.type));\
SPACE;\
ADD(field.name)

#define DECLARE_CONST_FIELD   \
if (checkFieldDecimalType(field.type)) {\
ADD(getFieldCppType(field.type));\
} else {\
ADD("const " + getFieldCppType(field.type) + "&");\
}\
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

QString AbstractGenerator::createDefaultConstruct() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (!field.default.isEmpty()) {
            TAB_2;
            ADD(field.name);
            EQUAL;
            if (checkFieldStrType(field.type)) {
                ADD_S(field.default);
            } else {
                ADD(field.default);
            }
            SEMICOLON;
            ENTER;
        }
    }
    TP_END;
}

QString AbstractGenerator::createConstructField() {
    TP_START;
    if (tb.fields.isEmpty()) {
        TP_END;
    }
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        TAB_2;
        DECLARE_CONST_FIELD;
        COMMA;
        ENTER;
    }
    USE_LEFT(2);
    TP_END;
}

QString AbstractGenerator::createConstructCommit() {
    TP_START;
    if (tb.fields.isEmpty()) {
        TP_END;
    }
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
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

QString AbstractGenerator::createFieldDeclare() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ENTER;
        TAB_2;
        ADD(QString("EntityField<%1> %2 = EntityField<%1>(\"%2\");")
            .arg(getFieldCppType(field.type))
            .arg(field.name)
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

QString AbstractGenerator::createFields() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        if (field.transient) {
            continue;
        }
        ENTER;
        TAB_4;
        ADD("<< \"");
        ADD(field.name);
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
        ADD(field.name);
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
        ADD(field.name);
        SPACE;
        ADD(getDatabaseFieldType(field.type));
        if (!field.default.isEmpty()) {
            SPACE;
            ADD("default ");
            if (checkFieldStrType(field.type)) {
                ADD_s(field.default);
            } else {
                ADD(field.default);
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
            }
        }
        ADD(getComment(field.note));
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
            ADD_S(field.name);
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
                ADD_S(s);
            }
            ADD(")");
        }
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
            ADD(QString("name == \"%1\"").arg(field.name));
        }
    }
    if (str.isEmpty()) {
        str = "false";
    }
    TP_END;
}

QString AbstractGenerator::createValuesGetWithoutAutoIncrement() {
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
        ADD("<< entity.");
        ADD(field.name);
    }
    TP_END;
}

QString AbstractGenerator::createGetValueByName() {
    TP_START;
    FIELD_FOREACH(tb.fields) {
        ADD("if (target == \"");
        ADD(field.name);
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
            break;
        }
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
        ADD(field.name);
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
        if (checkFieldDecimalType(field.type)) {
            DECLARE_FIELD;
        } else {
            DECLARE_CONST_FIELD;
        }
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
    header.replace("$SqlType$", getSqlNamespaceName());
    QString headerOutputFile = getOutputFilePath("EntityInclude.h");
    {
        QFile file(headerOutputFile);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.write(header.toLocal8Bit());
        file.close();
    }

    auto cpp = loadTemplateFile(":/generator/templates/entitydelegate_cpp.txt");
    cpp.replace("$SqlType$", getSqlNamespaceName());

    QString entityHeaders;
    QString entityListStr;
    for (const auto& name : tbNames) {
        entityHeaders.append(QString("#include \"%1.h\"\n").arg(name));
        entityListStr.append(name).append(",");
    }
    cpp.replace("$EntityHeaders$", entityHeaders);
    cpp.replace("$EntityList$", entityListStr.chopped(1));
    cpp.replace("$DbLoaderPath$", dbloadPath);

    QString cppOutputFile = getOutputFilePath("EntityInclude.cpp");
    {
        QFile file(cppOutputFile);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.write(cpp.toLocal8Bit());
        file.close();
    }
}
