//
// Created by user on 25-1-3.
//

#ifndef EXPR_H
#define EXPR_H
#include <stack>

#include "../include/parser.h"
#include "../include/table.h"

namespace iedb
{
    class expr
    {
    public:


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
        // static int expr_execute(const std::vector<instruct>& ins, int start_ins_offset,row & row_data, column_value & out_result);
    };
}

#endif //EXPR_H
