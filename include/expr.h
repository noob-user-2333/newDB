//
// Created by user on 25-1-3.
//

#ifndef EXPR_H
#define EXPR_H
#include <stack>

#include "parser.h"
#include "table.h"

namespace iedb {
    //对中缀表达式进行处理生成对应指令
    struct instruct
    {
        enum class op_type
        {
            add,
            sub,
            mul,
            div,
            mod,
            And,
            Or,
            less,
            less_equal,
            equal,
            not_equal,
            more,
            more_equal,
            load_imm,
            load_col
        };
        enum class data_type
        {
            Int,
            Float,
            Text
        };
        static const std::unordered_map<token_type,op_type>  token_to_op;
        op_type op;
        data_type type;
        union
        {
            long value_int;
            double value_float;
            long col_offset;
        };
        instruct() = delete;
        instruct(op_type op, data_type type,long value) : op(op), type(type), value_int(value) {}
        instruct(op_type op, data_type type, double value) : op(op), type(type), value_float(value) {}
        std::string to_string();
    };
    class expr {
    private:
        static const std::unordered_map<token_type,int> op_priority ;
        static int op_token_process(token_type type,int priority,std::stack<token_type>&stack, std::vector<instruct>& ins);
        static instruct token_to_imm_instruct(const token& token);
        static instruct token_to_col_instruct(const token& token, const col_def&col);
    public:

        //传入中缀表达式，返回处理后的后缀表达式的第一个元素,其继承传入的根节点的兄弟节点
        static token* convert_infix_to_suffix(token* root);

    };



}

#endif //EXPR_H
