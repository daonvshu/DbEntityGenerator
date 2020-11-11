#pragma once

#include <qobject.h>

#include "Entity.h"

enum SqlType {
    TYPE_SQLITE,
    TYPE_MYSQL,
    TYPE_SQLSERVER,
};

class ConfigLoader {
public:
    ConfigLoader(const QString& configPath);
    ~ConfigLoader();

    bool load();

    SqlType getSqlType() {
        return configType;
    }

    QString getCfgFilePath() {
        return cfgFilePath;
    }

    template<typename T>
    const T& getEntity() {
        return *static_cast<T*>(data);
    }

private:
    QString path;
    SqlType configType;
    Entity* data;
    QString cfgFilePath;
};
