//
// Created by user on 24-11-29.
//

#ifndef PARSER_H
#define PARSER_H
#include "token.h"

namespace iedb {
    struct parse_result {
        std::unique_ptr<std::vector<token>> tokens;
        token_type type;
        token* master;  //指向该子句的首个操作目标，例如 SELECT 子句指向第一列名，INSERT 子句指向第一插入值
        token* target;  //指向第一个from子句表达式
        token* filter;  //指向第一个where子句表达式
        token* group;   //指向第一个group子句表达式
        token* order;   //指向第一个order子句表达式
        token* limit;   //指向limit子句表达式
        explicit parse_result(std::unique_ptr<std::vector<token>> tokens):tokens(std::move(tokens)), type(token_type::space),master(nullptr),target(nullptr), filter(nullptr), group(nullptr), order(nullptr), limit(nullptr){}
        static std::unique_ptr<parse_result> parse(std::unique_ptr<std::vector<token>> tokens);
        void print();

    private:
        static void print_token_list(token * first);
    };
    //默认单线程解析，同时仅会运行一个解析函数
    std::unique_ptr<parse_result> parse(const char *sql);
};

#endif //PARSER_H
