#include "db.h"
#include "db_data_manager.h"
#include "parser.h"
#include "test/test.h"
#include <chrono>
#include <iostream>
#include <ostream>
using namespace iedb;
constexpr char path[] = "/dev/shm/iedb.db";
constexpr char create[] = "CREATE TABLE test("
                          "id int,"
                          "score float,"
                          "name text);";
constexpr char insert[] = "INSERT INTO test VALUES (0x12,4.345,'gooddafsafsdfsdf');";
constexpr char query[] = "SELECT * FROM test where  1 > 2 < 3 and 2 -4 > 5 or 3 - 2 * 6 < 5 > 1;";
int main() {
    auto db = db::open(path);
    auto sqls = test::get_sql_from_file("/dev/shm/apple_stock.sql");
    auto start = std::chrono::high_resolution_clock::now();
    for (auto&sql:sqls)
        db->sql_execute_without_reader(sql.c_str());
    // 获取结束时间
    auto end = std::chrono::high_resolution_clock::now();
    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "代码执行耗时: " << duration.count() << " 微秒" << std::endl;
    return 0;
}
