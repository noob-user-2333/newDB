#include "dbManager.h"
#include "newDB.h"
#include "vdbe_stmt.h"
#include "test/test.h"
using namespace iedb;
char path[1024] = "/dev/shm/apple_stock.sql";
char original_buffer[0x1000];
char buffer[0x2000];
int64 a = -1;
uint64 b = 2;
std::array<uint8,1024 * 1024> test1;
char expr[] = "select (a + 2.5) * (b - 3) / (c % 5 + 1) + d * 1.2 - (a / (b + 1.0)) + (c * d) / (a + 1) from test;";
std::string sql1 = "select * from test where id >= 5;";
std::string sql =
        "SELECT "
        "reading_value * 1.05 + 10 - reading_value * 0.05 - 10 * adjusted_value, "
        "reading_value * 2 + reading_value * 3 + reading_value * 4 * weighted_sum, "
        "reading_value - (reading_value / 2) * 2 + (reading_value / 2) * 2 + value_check, "
        "reading_value * 10 / 10  + value_identity "
        "FROM sensor_data "
        "WHERE "
        "reading_value > 10 AND reading_value < 500 "
        "GROUP BY id, device_id, sensor_type, reading_value, reading_time;";
int main() {

    auto sqls = test::get_sql_from_file(path);
    auto time = new timer();
    time->start_timing();
    for (auto & sql:sqls)
        IEDB_execute_sql_without_reader(sql.c_str());
    time->end_timing();
    std::cout << "代码执行耗时: " << time->cost_time_for_us() << " 微秒" << std::endl;
    delete time;
    // 以下是对sqlite3的示例代码
    // test::sqlite_test(sqls);
    // sqlite3 *db;
    // assert(test::run() == 0);
    // test::run();
    return 0;
}
