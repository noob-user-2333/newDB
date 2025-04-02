//
// Created by user on 25-4-1.
//

#ifndef TOOL_H
#define TOOL_H
#include "newDB.h"
#include "sqlite3.h"


namespace iedb {

    void ImportDataFromSqlite(sqlite3* db);

    void ExportDataToSqlite(sqlite3* db);






}




#endif //TOOL_H
