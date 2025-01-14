//
// Created by user on 25-1-14.
//

#ifndef TEST_H
#define TEST_H
#include <fstream>
#include <iostream>

#include "utility.h"

namespace iedb
{
    class test
    {
    public:
        static std::vector<std::string> get_sql_from_file(const char * file_path)
        {
            std::vector<std::string> lines;
            std::ifstream file(file_path);  // 打开文件

            if (!file.is_open()) {  // 检查文件是否成功打开
                std::cerr << "无法打开文件: " << file_path << std::endl;
                return lines;
            }

            std::string line;
            while (getline(file, line)) {  // 按行读取文件内容
                lines.push_back(line);
            }

            file.close();  // 关闭文件
            return lines;
        }


    };



}

#endif //TEST_H
