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

void sqlite_test(std::vector<std::string> & sqls)
{
    sqlite3 *db;
    auto status = sqlite3_open("/dev/shm/sqlite.db",&db);
    auto start = std::chrono::high_resolution_clock::now();
    for (auto & sql:sqls)
    {
        sqlite3_exec(db,sql.c_str(),nullptr,nullptr,nullptr);
    }// 获取结束时间
    auto end = std::chrono::high_resolution_clock::now();
    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "代码执行耗时: " << duration.count() << " 微秒" << std::endl;
    sqlite3_close_v2(db);
}

auto num =
    1.0;
int main() {
    auto data = (char*)&num;
    for (auto i = 0; i < 8;i++)
        printf("%d ",data[i]);
    // auto db = db::open(path);
    // auto sqls = test::get_sql_from_file("/dev/shm/Games.sql");
    // auto start = std::chrono::high_resolution_clock::now();
    // for (auto&sql:sqls)
    //     db->sql_execute_without_reader(sql.c_str());
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "代码执行耗时: " << duration.count() << " 微秒" << std::endl;
    // sqlite_test(sqls);
    return 0;
}
