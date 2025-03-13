//
// Created by user on 25-3-11.
//
// #include "../docs/parse.c"

#include "AST.h"
#include "../docs/parse.c"

namespace iedb {
    std::unique_ptr<AST> AST::parser(std::unique_ptr<std::vector<token> > &tokens) {
        auto result = std::unique_ptr<AST>(new AST(tokens));
        result->type = token_type::select;
        int status_code = 0;
        auto& ref_tokens = result->tokens;
        yyParser parse;
        ParseInit(&parse);
        auto i = 0;
        do{
            Parse(&parse,static_cast<int>((*ref_tokens)[i].type),&(*ref_tokens)[i],result.get());
            i++;
        }while (i < ref_tokens->size() && result->type != token_type::error);
        Parse(&parse,0,nullptr,result.get());
        if (token_type::error != result->type)
            return result;
        return nullptr;
    }
    std::unique_ptr<AST> AST::parse(const char *sql) {
        auto tokens = token::lexer(sql);
        if (tokens)
            return parser(tokens);
        return nullptr;
    }

    void AST::print_expr(token *root) {
        //使用后序遍历
        if (root->left)
            print_expr(root->left);
        if (root->right)
            print_expr(root->right);
        printf("%s ",root->to_string().c_str());
    }
















}
