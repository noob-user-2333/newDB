#include "dbManager.h"
#include "DBreader.h"
#include "newDB.h"
#include "vdbe_cursor.h"
#include "test/test.h"
#include "test/timer.h"
// #include "test/test.h"
using namespace iedb;
char dbPath[] = "/dev/shm/IEDB_V3/test.db";
char path[1024] = "/dev/shm/apple_stock.sql";
char original_buffer[0x1000];
char buffer[0x2000];
char sql[] = "select Low+High,Open from apple_stock where Open > 0.12  group by High + Low order by Low,Open;";
char sql1[] = "select Low+High,Open from apple_stock where Open > 0.12  group by High + Low order by Low,Open,rowid;";
sqlite3 *db;
sqlite3_stmt *stmt;

int main() {
    auto p = IEDB_reader_malloc();
    std::vector<uint8> record;
    int index = 0;
    assert(sqlite3_open(dbPath,&db) == SQLITE_OK);
    // auto sqls = test::get_sql_from_file(path);
    // for (auto & sql:sqls) {
    // IEDB_execute_sql_without_reader(sql.c_str());
    // sqlite3_exec(db,sql.c_str(),nullptr,nullptr,nullptr);
    // }

    IEDB_execute_query_sql(sql, p);
    assert(sqlite3_prepare_v2(db,sql1,sizeof(sql1),&stmt,nullptr) == SQLITE_OK);
    auto time = new timer();
    time->start_timing();
    while (p->next() == status_ok) {
        assert(sqlite3_step(stmt) == SQLITE_ROW);
        auto p1 = p->get_column_double(0);
        auto p2 = p->get_column_double(1);
        auto p3 = sqlite3_column_double(stmt, 0);
        auto p4 = sqlite3_column_double(stmt, 1);
        assert(std::abs(p1 - p3) < 0.00001);
        assert(std::abs(p2 - p4) < 0.00001);
        index++;
    }

    time->end_timing();
    time->printf();
    // delete time;
    // 以下是对sqlite3的示例代码
    // test::sqlite_test(sqls);
    // sqlite3 *db;
    // assert(test::run() == 0);
    // test::run();
    IEDB_reader_free(p);
    return 0;
}
