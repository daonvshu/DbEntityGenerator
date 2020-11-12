#pragma once

#include "Entity.h"

class AbstractGenerator {
public:
    AbstractGenerator(QString outputPath);

    virtual void generate() = 0;

protected:
    QString outputPath;
    Table tb;
    int currentFieldSize;

protected:
    QString getOutputFilePath(const Table& table);
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

    void setCurrentTable(const Table& tb) {
        this->tb = tb;
    }

    QString createFieldList();
    QString createDefaultConstruct();
    QString createConstructField();
    QString createConstructCommit();
    QString createFieldSize();
    QString createTableName(const QString& prefix);
    QString createDatabaseType();
    QString createPrimaryKeys();
    QString createIndexFields(QString indexType = "index");
    QString createSetterGetter();
    QString createMetaType();
};
