//
// Created by user on 25-3-11.
//

#ifndef AST_H
#define AST_H
#include "token.h"


namespace iedb {
    //语法树
    class AST {

    private:
        explicit AST(std::unique_ptr<std::vector<token>>&tokens):tokens(std::move(tokens)),type(token_type::error),target_table(nullptr){}
        static std::unique_ptr<AST> parser(std::unique_ptr<std::vector<token>>&tokens);
    public:
        std::unique_ptr<std::vector<token>> tokens;
        token_type type;
        token * target_table;
        token* master{}; //对于select表示输出表达式，对于create 和 insert则表示列定义和数据
        token* where{};   //筛选数据用表达式
        token* order{};
        token* group{};
        token* limit{};
        static std::unique_ptr<AST> parse(const char*sql);
        static void print_expr(token* root);
    };










}

#endif //AST_H
