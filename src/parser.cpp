//
// Created by user on 24-12-1.
//
#include "parser.h"
#include "parse.c"
namespace iedb {
    std::unique_ptr<parse_result> parse_result::parse(std::unique_ptr<std::vector<token>>  tokens) {
        auto result = std::make_unique<parse_result>(std::move(tokens));
        int status_code = 0;
        auto& ref_tokens = result->tokens;
        yyParser parse;
        ParseInit(&parse);
        for (auto i = 0; i < ref_tokens->size() && result->type != token_type::error;i++) {
            Parse(&parse,static_cast<int>((*ref_tokens)[i].type),&(*ref_tokens)[i],result.get());
        }
        Parse(&parse,0,nullptr,result.get());
        if (token_type::error != result->type)
            return result;
        return nullptr;
    }
    std::unique_ptr<parse_result> parse(const char * sql) {
        if (auto tokens = token::lexer(sql))
            return parse_result::parse(std::move(tokens));
        return nullptr;
    }
    void parse_result::print() {
        printf("parse statement(type is %d):%s\n",type,(*tokens)[0].sql);
        printf("master:\n");
        print_token_list(master);
        printf("target:\n");
        print_token_list(target);
        printf("filter:\n");
        print_token_list(filter);
        printf("group:\n");
        print_token_list(group);
        printf("order:\n");
        print_token_list(order);
        printf("limit:\n");
        print_token_list(limit);
    }



    void parse_result::print_token_list(token * first) {
        while (first) {
            for (auto f = first;f;f = f->child){
                std::string str(f->sql,f->offset,f->len);
                printf(" %s ",str.c_str());
            }
            first = first->brother;
            printf("\n");
        }
    }

};



