#include "dbManager.h"
#include "DBreader.h"
#include "newDB.h"
#include "vdbe_cursor.h"
#include "test/test.h"
#include "test/timer.h"
// #include "test/test.h"
/*
 *  TODO:完成工具用于从sqlite导入数据到IEDB
 *  TODO：完成工具用于分别从sqlite和IEDB读取数据并比较结果是否一致
 */
using namespace iedb;
char dbPath[] = "/dev/shm/test.db";
char path[1024] = "/dev/shm/apple_stock.sql";
char original_buffer[0x1000];
char buffer[0x2000];
char sql[] = "select Low+High,Open from apple_stock where Open > 0.12  group by High + Low order by Low,Open;";
char sql1[] = "select Low+High,Open from apple_stock where Open > 0.12  group by High + Low order by Low,Open,rowid;";
sqlite3 *db;
sqlite3_stmt *stmt;

int main() {
    std::vector<uint8> record;
    DBreader *p;
    assert(sqlite3_open(dbPath,&db) == SQLITE_OK);
    auto time = new timer();
    time->start_timing();
    auto sqls = test::get_sql_from_file(path);
    auto insertCount = sqls.size() - 1;

    sqlite3_exec(db,"PRAGMA journal_mode = PERSIST;",nullptr,nullptr,nullptr);
    sqlite3_exec(db,sqls[0].c_str(),nullptr,nullptr,nullptr);
    // IEDB_execute_sql_without_reader(sqls[0].c_str());
    for (auto count = 0; count < 5;count++) {
        for (auto times = 1;times < sqls.size();times++) {
            sqlite3_exec(db,sqls[times].c_str(),nullptr,nullptr,nullptr);
            // IEDB_execute_sql_without_reader(sqls[times].c_str());
        }
        printf("have %ld records,insert into %ld records\n",insertCount * count,insertCount);
        time->end_timing();
        time->printf();
    }
    delete time;
    IEDB_reader_free(p);
    return 0;
}
