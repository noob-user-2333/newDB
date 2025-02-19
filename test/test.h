//
// Created by user on 25-1-14.
//

#ifndef TEST_H
#define TEST_H
#include <fstream>
#include <iostream>
#include "pager.h"
#include "timer.h"
#include "utility.h"
#include <gtest/gtest.h>
#include "../third-part/sqlite/sqlite3.h"
namespace iedb
{
    class test
    {
    public:
        static void get_random(void *buffer,int64 size);
        static std::vector<std::string> get_sql_from_file(const char* file_path);
        static void sqlite_test(std::vector<std::string>& sqls);
        static void save_data_to_file(const void*data,int64 size,const char* file_path);
        static int run();


    };
}

#endif //TEST_H
