//
// Created by user on 24-11-25.
//

#ifndef TOKEN_H
#define TOKEN_H
#include "utility.h"
namespace iedb {
    enum class token_type{
        error = 0,
        //通用
        space,
        string,
        hex,
        number_int,
        number_float,
        name,
        //符号
        comma,
        semi,
        dot,
        parenthesis_left,
        parenthesis_right,
        bracket_left,
        bracket_right,
        brace_left,
        brace_right,
        //运算符
        plus,
        minus,
        star,
        slash,
        more_equal,
        less_equal,
        not_equal,
        more,
        less,
        bang,
        equal,
        bit_or,
        bit_and,
        //关键字
        Not,
        And,
        Or,
        select,
        create,
        update,
        Delete,
        from,
        where,
        order,
        group,
        by
    };

    struct token {
        const token_type type;
        const uint32 offset;
        const uint32 len;
        const char* sql;
        token * brother;
        token * child;
        token(token_type type,uint32 offset,uint32 len,const char* sql):type(type),offset(offset),len(len),sql(sql),brother(nullptr),child(nullptr){}
        void print();
        static std::unique_ptr<std::vector<token>> lexer(const char * sql);
    };



};
#endif //TOKEN_H
