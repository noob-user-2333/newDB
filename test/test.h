//
// Created by user on 25-1-14.
//

#ifndef TEST_H
#define TEST_H
#include <fstream>
#include <iostream>

#include "timer.h"
#include "utility.h"
#include "../third-part/sqlite/sqlite3.h"
namespace iedb
{
    class test
    {
    public:
        static std::vector<std::string> get_sql_from_file(const char* file_path)
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

        static void sqlite_test(std::vector<std::string>& sqls)
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
    };
}

#endif //TEST_H
