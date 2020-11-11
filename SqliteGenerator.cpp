#include "SqliteGenerator.h"

#include <qcryptographichash.h>
#include <qfile.h>
#include <qregexp.h>

SqliteGenerator::SqliteGenerator(QString outputPath, SqliteEntity entity)
    : outputPath(outputPath)
    , entity(entity)
{
}

void SqliteGenerator::generate() {
    QString hppTemplate = loadTemplateFile(":/generator/templates/sqlite.txt");
    for (const auto& tb : entity.tables) {
        auto header = hppTemplate;
        //set classname
        header.replace("$ClassName$", tb.name);
        //set member list

    }
}

QString SqliteGenerator::getOutputFilePath(const Table& table) {
    return QString("%1/%2.h").arg(outputPath).arg(table.name);
}

QByteArray SqliteGenerator::getFileMd5(const QString& filePath) {
    QFile file(filePath);
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

QString SqliteGenerator::loadTemplateFile(const QString& name) {
    QFile file(name);
    file.open(QIODevice::ReadOnly);
    auto data = file.readAll();
    file.close();
    return data;
}