//
// Created by user on 24-11-26.
//

#ifndef LEXER_H
#define LEXER_H
#include <utility>
#include "Token.h"
namespace iedb {
    struct lexer_result {
        explicit lexer_result(const char *sql,std::vector<token> tokens):tokens(std::move(tokens)),sql(sql){}
        const std::vector<token> tokens;
        const std::string  sql;
        void print();
        static std::unique_ptr<lexer_result> lexer(const char * sql);
    };





};
#endif //LEXER_H
