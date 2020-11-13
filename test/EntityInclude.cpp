#include "EntityInclude.h"
#include "Test1.h"
#include "Test2.h"

#include "DbLoader.h"
namespace DaoSqlite {
    void EntityDelegate::createEntityTables() {
        DbLoader::getClient().createTables<Test1,Test2>();
    }

    const int entityDelegateId = qRegisterMetaType<EntityDelegate*>();
}

