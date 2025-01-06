//
// Created by user on 25-1-3.
//
#include "expr.h"

#include <stack>

namespace iedb {
    token* expr::convert_infix_to_suffix(token *root) {
        static std::stack<token*> stack;
        assert(stack.empty());
        const auto next_brother = root->brother;



        return root;
    }



}