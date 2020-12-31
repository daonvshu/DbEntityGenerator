#pragma once

#include "Entity.h"

#include "AbstractGenerator.h"

class MysqlGenerator : public AbstractGenerator {
public:
    MysqlGenerator(QString outputPath, Entity entity, QString dbloadPath);

    void generate();

private:
    Entity entity;

protected:
    QString getFieldCppType(const QString& fieldType);
    bool checkFieldStrType(const QString& fieldType);
    bool checkFieldDecimalType(const QString& fieldType);
    QString getDatabaseFieldType(const QString& fieldType);
    QString getComment(const QString& note);
    QString getAutoIncrementStatement();

    QString getSqlNamespaceName();
};