#pragma once

#include "Entity.h"

class SqliteGenerator {
public:
    SqliteGenerator(QString outputPath, SqliteEntity entity);

    void generate();

private:
    QString outputPath;
    SqliteEntity entity;

private:
    QString getOutputFilePath(const Table& table);
    QByteArray getFileMd5(const QString& filePath);
    QString loadTemplateFile(const QString& name);

};