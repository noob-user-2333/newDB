#include "db.h"
#include "db_data_manager.h"
#include "parser.h"
#include "test/test.h"
#include "test/timer.h"
#include <iostream>
#include <ostream>
using namespace iedb;
constexpr char path[] = "/dev/shm/iedb.db";
constexpr char create[] = "CREATE TABLE test("
                          "id int,"
                          "score float,"
                          "name text);";
constexpr char insert[] = "INSERT INTO test VALUES (0x12,4.345,'gooddafsafsdfsdf');";
constexpr char query[] = "SELECT * FROM Games where  gameId + attendance - (5 + 3) /2   > 40400753;";



auto num =
    1.0;
sqlite3 *sqlite = nullptr;
sqlite3_stmt * stmt;
int main() {
    auto status = sqlite3_open("/dev/shm/sqlite.db", &sqlite);
    status = sqlite3_prepare(sqlite,query, sizeof(query), &stmt, nullptr);
    auto db = db::open(path);
    // for (auto& sql : sqls)
        // db->sql_execute_without_reader(sql.c_str());
    auto reader = db->sql_execute(query);
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int64 num;
        reader->next();
        auto sqlite_num = sqlite3_column_int64(stmt,0);
        status = reader->get_int_value(0, num);
        if (sqlite_num != num)
        {
            printf("error: sqlite(%lld)!= iedb(%lld)\n", sqlite_num, num);
        }
    }
    return 0;
}
