//
// Created by user on 25-1-3.
//

#ifndef EXPR_H
#define EXPR_H
#include <stack>

#include "parser.h"
#include "table.h"

namespace iedb
{
    //TODO:暂不支持string的比较功能
    class expr
    {
    public:
        //对中缀表达式进行处理生成对应指令
        struct instruct
        {
            //如果指令翻译过程中存在错误，则op_type = error ,value_int设置为错误码
            enum class op_type
            {
                error,
                add,
                minus,
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
                load_col,
                end
            };
            static const std::unordered_map<token_type, op_type> token_to_op;
            op_type op;
            column_data_type data;
            instruct() = delete;
            explicit instruct(op_type op) : op(op),data(0){}
            instruct(op_type op, long value) : op(op),data(value){}
            instruct(op_type op, double value) : op(op),data(value){}
            instruct(op_type op, const char *start,int len) : op(op), data(std::string(start,len)){}

            static instruct get_end_instruct();
        };



    private:
        static const std::unordered_map<token_type, int> op_priority;
        static instruct token_to_instruct(const token& node, const table& target_table);
    public:
        //传入中缀表达式，返回处理后的后缀表达式的第一个元素,其继承传入的根节点的兄弟节点
        static token* convert_infix_to_suffix(token* root);
        //传入表达式，将其转化为指令格式,转化完毕后会在末尾添加end指令
        static int convert_expr_to_instruct(token* expr_root, const table& target_table, std::vector<instruct>& ins);
        //生成对指定表中的每一列进行方位的 vector<ins> ,用于处理select子句为*的情况
        static void select_statement_of_star_process(const table& target_table, std::vector<instruct>& ins);
        //传入生成的表达式指令数组和起始指令偏移量、运算栈、数据，当遇到end指令后运算结束
        static int expr_execute(const std::vector<instruct>& ins, int start_ins_offset,row & row_data, column_data_type & out_result);
    };
}

#endif //EXPR_H
