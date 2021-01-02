#pragma once

#include "Entity.h"

class AbstractGenerator {
public:
    AbstractGenerator(QString outputPath, QString dbloadPath);

    virtual void generate() = 0;

protected:
    QString outputPath, dbloadPath;
    Table tb;
    int currentFieldSize;
    int currentPrimaryKeySize;

protected:
    QString getOutputFilePath(const Table& table);
    QString getOutputFilePath(const QString& fileName);
    QByteArray getFileMd5(const QString& filePath);
    QString loadTemplateFile(const QString& name);
    QString lowerAndSplitWithUnderline(const QString& s);
    QString upperFirstChar(const QString& s);

    void writeTableHeaderByDiff(const QString& content, const Table& tb);

    virtual QString getFieldCppType(const QString& fieldType) = 0;
    virtual QString getCppDefaultValueString(const QString& fieldType, const QString& defaultValue) = 0;
    virtual QString getDatabaseDefaultValueString(const QString& fieldType, const QString& defaultValue) = 0;
    virtual QString getDatabaseFieldType(const QString& fieldType) = 0;
    virtual QString getComment(const QString& note) = 0;
    virtual QString getAutoIncrementStatement() = 0;

    virtual QString getSqlNamespaceName() = 0;

    void setCurrentTable(const Table& tb) {
        this->tb = tb;
    }

    QString createFieldList();
    QString createConstruct();
    QString createDefaultFieldInit();
    QString createConstructField(bool skipDefaultValue = false);
    QString createConstructCommit(bool skipDefaultValue = false);
    QString createFieldDeclare(const QString& prefix);
    QString createFieldDeclareReset();

    QString createFieldSize();
    QString createTableName(const QString& prefix);
    QString createTableEngine(const QString& engine);
    QString createFields();
    QString createFieldsWithoutAutoIncrement();
    QString createDatabaseType();
    QString createPrimaryKeys();
    QString createIndexFields(QString indexType = "index");
    QString createCheckNameIncrement();

    QString createValuesGetWithoutAutoIncrement();
    QString createGetValueByName();
    QString createBindAutoIncrementId();
    QString createBindValue();

    QString createSetterGetter();
    QString createMetaType();

    void generateEntityDelegate(QStringList tbNames);

    void writeUtf8ContentWithBomHeader(const QString& path, const QString& content);
};
