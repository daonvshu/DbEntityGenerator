#pragma once

#include "Entity.h"

#include "AbstractGenerator.h"

class SqlServerGenerator : public AbstractGenerator {
public:
    SqlServerGenerator(QString outputPath, Entity entity, QString dbloadPath);

    void generate();

private:
    Entity entity;

protected:
    QString getFieldCppType(const QString& fieldType) override;
    QString getCppDefaultValueString(const QString& fieldType, const QString& defaultValue) override;
    QString getDatabaseDefaultValueString(const QString& fieldType, const QString& defaultValue) override;
    QString getDatabaseFieldType(const QString& fieldType) override;
    QString getComment(const QString& note) override;
    QString getAutoIncrementStatement() override;

    QString getSqlTypeName() override;
    QString getSqlClientTypeName() override;
};