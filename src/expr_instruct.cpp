//
// Created by user on 25-1-15.
//
#include "expr.h"

namespace iedb
{
    const std::unordered_map<token_type, expr::instruct::op_type> expr::instruct::token_to_op = {
        {token_type::plus, op_type::add},
        {token_type::minus, op_type::sub},
        {token_type::star, op_type::mul},
        {token_type::slash, op_type::div},
        {token_type::percent, op_type::mod},
        {token_type::And, op_type::And},
        {token_type::Or, op_type::Or},
        {token_type::less, op_type::less},
        {token_type::less_equal, op_type::less_equal},
        {token_type::equal, op_type::equal},
        {token_type::not_equal, op_type::not_equal},
        {token_type::more, op_type::more},
        {token_type::more_equal, op_type::more_equal},
        {token_type::number_float, op_type::load_imm_float},
        {token_type::number_int, op_type::load_imm_int},
        {token_type::string, op_type::load_imm_string},
        {token_type::name, op_type::load_col_int}
    };
    expr::instruct expr::instruct::get_end_instruct()
    {
        return {instruct::op_type::end, 0L};
    }
    expr::instruct expr::token_to_instruct(const token& node, const table& target_table)
    {
        auto start = node.sql + node.offset;
        auto len = node.len;
        auto end = start + len;
        auto base = 10;
        char* str_end = nullptr;
        switch (node.type)
        {
        case token_type::hex:
            {
                start += 2;
                base = 16;
            }
        case token_type::number_int:
            {
                auto value = strtol(start, &str_end, base);
                assert(str_end == end);
                return {instruct::op_type::load_imm_int,value};
            }
        case token_type::number_float:
            {
                auto value = std::strtod(start, &str_end);
                assert(str_end == end);
                return {instruct::op_type::load_imm_float, value};
            }
        case token_type::name:
            {
                auto col = target_table.get_col_by_name(node.to_string());
                if (col == nullptr)
                    return {
                        instruct::op_type::error, static_cast<long>(status_not_find_column)
                    };
                int64 offset = col->line_offset;
                //根据列类型决定操作
                switch (col->type)
                {
                case column_type::Int:
                    return {instruct::op_type::load_col_int, offset};
                case column_type::text:
                    return {instruct::op_type::load_col_string, offset};
                case column_type::Float:
                    return {instruct::op_type::load_col_float, offset};
                default:
                    throw std::runtime_error("should not happen in expr::convert_token_to_instruct");
                }
                return {instruct::op_type::error, static_cast<long>(status_error)};
            }
        default:
            {
                //此处用于处理运算符
                auto it = instruct::token_to_op.find(node.type);
                if (it != instruct::token_to_op.end())
                    return {it->second,0L};
                throw std::runtime_error("should never happen for token_to_imm_instruct");
            }
        }
    }




}
