//
// Created by user on 25-2-14.
//
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

#include "test.h"

namespace iedb
{
    void test::get_random(void* buffer, int64 size)
    {
        auto f = fopen("/dev/random", "r");
        assert(f);
        fread(buffer, size, 1, f);
        fclose(f);
    }

    std::vector<std::string> test::get_sql_from_file(const char* file_path)
    {
        std::vector<std::string> lines;
        std::ifstream file(file_path); // 打开文件

        if (!file.is_open())
        {
            // 检查文件是否成功打开
            std::cerr << "无法打开文件: " << file_path << std::endl;
            return lines;
        }

        std::string line;
        while (getline(file, line))
        {
            // 按行读取文件内容
            lines.push_back(line);
        }

        file.close(); // 关闭文件
        return lines;
    }

    void test::sqlite_test(std::vector<std::string>& sqls)
    {
        timer timer;
        sqlite3* db;
        auto status = sqlite3_open("/dev/shm/sqlite.db", &db);
        timer.start_timing();
        for (auto& sql : sqls)
            sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
        timer.end_timing();
        std::cout << "代码执行耗时: " << timer.cost_time_for_us() << " 微秒" << std::endl;
        sqlite3_close_v2(db);
    }
    void test::save_data_to_file(const void* data, int64 size, const char* file_path)
    {
        auto f = fopen(file_path, "w");
        assert(f);
        assert(fwrite(data, 1,size, f) == size);
        fclose(f);
    }

    int test::run()
    {
        testing::InitGoogleTest();
        return RUN_ALL_TESTS();
    }

}
