#pragma once

#include "Entity.h"

#include "AbstractGenerator.h"

class SqliteGenerator : public AbstractGenerator {
public:
    SqliteGenerator(QString outputPath, SqliteEntity entity, QString dbloadPath);

    void generate();

private:
    QString outputPath;
    SqliteEntity entity;

protected:
    QString getFieldCppType(const QString& fieldType);
    bool checkFieldStrType(const QString& fieldType);
    bool checkFieldDecimalType(const QString& fieldType);
    QString getDatabaseFieldType(const QString& fieldType);
    QString getComment(const QString& note);
    QString getAutoIncrementStatement();

    QString getSqlNamespaceName();
};