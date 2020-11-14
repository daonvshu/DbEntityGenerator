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
    virtual bool checkFieldStrType(const QString& fieldType) = 0;
    virtual bool checkFieldDecimalType(const QString& fieldType) = 0;
    virtual QString getDatabaseFieldType(const QString& fieldType) = 0;
    virtual QString getComment(const QString& note) = 0;
    virtual QString getAutoIncrementStatement() = 0;

    virtual QString getSqlNamespaceName() = 0;

    void setCurrentTable(const Table& tb) {
        this->tb = tb;
    }

    QString createFieldList();
    QString createDefaultConstruct();
    QString createConstructField();
    QString createConstructCommit();
    QString createFieldDeclare();
    QString createFieldSize();
    QString createTableName(const QString& prefix);
    QString createFields();
    QString createDatabaseType();
    QString createPrimaryKeys();
    QString createIndexFields(QString indexType = "index");
    QString createSetterGetter();
    QString createMetaType();

    void generateEntityDelegate(QStringList tbNames);
};