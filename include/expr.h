//
// Created by user on 25-1-3.
//

#ifndef EXPR_H
#define EXPR_H
#include "parser.h"

namespace iedb {
    //将中缀表达式转化为后缀表达式
    //根据
    class expr {
        public:
        //传入中缀表达式，返回处理后的后缀表达式的第一个元素
        static token* convert_infix_to_suffix(token * root);

    };



}

#endif //EXPR_H
