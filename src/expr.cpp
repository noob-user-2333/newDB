//
// Created by user on 25-1-3.
//
#include "expr.h"


namespace iedb
{
    const std::unordered_map<token_type, instruct::op_type> instruct::token_to_op = {
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
        {token_type::number_float, op_type::load_imm},
        {token_type::number_int, op_type::load_imm},
        {token_type::string, op_type::load_imm},
        {token_type::name, op_type::load_col}
    };
    const std::unordered_map<token_type, int> expr::op_priority = {
        {token_type::parenthesis_left, 100},
        {token_type::star, 3},
        {token_type::slash, 3},
        {token_type::percent, 3},
        {token_type::plus, 4},
        {token_type::minus, 4},
        {token_type::less, 6},
        {token_type::less_equal, 6},
        {token_type::more, 6},
        {token_type::more_equal, 6},
        {token_type::equal, 7},
        {token_type::not_equal, 7},
        {token_type::And, 11},
        {token_type::Or, 12}
    };

    std::string instruct::to_string()
    {
        static const std::string map[] = {
            "add",
            "sub",
            "mul",
            "div",
            "mod",
            "And",
            "Or",
            "less",
            "less_equal",
            "equal",
            "not_equal",
            "more",
            "more_equal",
            "load_imm",
            "load_col"
        };
        return map[static_cast<int>(op)];
    }
    instruct instruct::get_end_instruct()
    {
        return {instruct::op_type::end, instruct::data_type::Int, 0L};
    }

    instruct expr::token_to_instruct(const token& node, const table& target_table)
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
                return {instruct::op_type::load_imm, instruct::data_type::Int, value};
            }
        case token_type::number_float:
            {
                auto value = std::strtod(start, &str_end);
                assert(str_end == end);
                return {instruct::op_type::load_imm, instruct::data_type::Float, value};
            }
        case token_type::name:
            {
                auto col = target_table.get_col_by_name(node.to_string());
                if (col == nullptr)
                    return {
                        instruct::op_type::error, instruct::data_type::Int, static_cast<long>(status_not_find_column)
                    };
                int64 offset = col->line_offset;
                instruct::op_type op = instruct::op_type::load_col;
                instruct::data_type data_type;
                //根据列类型决定操作
                switch (col->type)
                {
                case column_type::Int:
                    return {op, instruct::data_type::Int, offset};
                case column_type::text:
                    return {op, instruct::data_type::Text, offset};
                case column_type::Float:
                    return {op, instruct::data_type::Float, offset};
                default:
                    throw std::runtime_error("should not happen in expr::convert_token_to_instruct");
                }
                return {instruct::op_type::error, instruct::data_type::Int, static_cast<long>(status_error)};
            }
        default:
            {
                //此处用于处理运算符
                auto it = instruct::token_to_op.find(node.type);
                if (it != instruct::token_to_op.end())
                    return {it->second, instruct::data_type::Int, 0L};
                throw std::runtime_error("should never happen for token_to_imm_instruct");
            }
        }
    }




    token* expr::convert_infix_to_suffix(token* root)
    {
        static std::stack<token*> stack;
        static std::queue<token*> suffixQueue;
        assert(root && stack.empty() && suffixQueue.empty());
        auto brother = root->brother;
        for (auto node = root; node; node = node->child)
        {
            switch (node->type)
            {
            //算数类单词直接入队
            case token_type::name:
            case token_type::number_float:
            case token_type::number_int:
            case token_type::hex:
            case token_type::string:
                {
                    suffixQueue.push(node);
                    break;
                }
            //遭遇左括号直接入栈
            case token_type::parenthesis_left:
                {
                    stack.push(node);
                    break;
                }
            //遭遇右括号则一直出栈，直到遇到第一个左括号
            case token_type::parenthesis_right:
                {
                    while (stack.top()->type != token_type::parenthesis_left)
                    {
                        suffixQueue.push(stack.top());
                        stack.pop();
                    }
                    stack.pop();
                    break;
                }
            //当传入token为运算符时
            default:
                {
                    auto it = op_priority.find(node->type);
                    //不应当存在非运算符单词
                    if (it == op_priority.end())
                    {
                        char buff[4096];
                        snprintf(buff, sizeof(buff), "invalid token for %s\n", node->to_string().c_str());
                        throw std::runtime_error(buff);
                    }
                    int priority = it->second;
                    //开始处理
                    //在栈非空时，需确保新插入元素优先级大于栈顶元素
                    while (stack.empty() == false)
                    {
                        auto top = stack.top();
                        auto top_priority = op_priority.find(top->type)->second;
                        if (priority >= top_priority)
                        {
                            suffixQueue.push(stack.top());
                            stack.pop();
                        }
                        else
                            break;
                    }
                    stack.push(node);
                }
            }
        }
        //将stack中的运算符全部push
        while (stack.empty() == false)
        {
            suffixQueue.push(stack.top());
            stack.pop();
        }
        //将suffixQueue中的token全部pop到root
        auto new_root = suffixQueue.front();
        auto current = suffixQueue.front();
        suffixQueue.pop();
        while (suffixQueue.empty() == false)
        {
            current->child = suffixQueue.front();
            current = current->child;
            suffixQueue.pop();
        }
        current->child = nullptr;
        new_root->brother = brother;
        return new_root;
    }
    int expr::convert_expr_to_instruct(token* expr_root, const table& target_table, std::vector<instruct>& ins)
    {
        for (auto node = expr_root; node; node = node->child)
        {
            if (node->type == token_type::parenthesis_left || node->type == token_type::parenthesis_right)
                throw std::runtime_error("parenthesis should not be in expression");
            auto new_ins = token_to_instruct(*node, target_table);
            if (new_ins.op == instruct::op_type::error)
                return static_cast<int>(new_ins.value_int);
            ins.push_back(new_ins);
        }
        ins.emplace_back(instruct::op_type::end, instruct::data_type::Int, 0L);
        return status_ok;
    }

    void expr::select_statement_of_star_process(const table& target_table, std::vector<instruct>& ins)
    {
        auto count = target_table.get_col_count();
        for (auto i = 0; i < count;i++)
        {
            auto col = target_table.get_col_by_index(i);
            auto offset = static_cast<int64>(col->line_offset);
            instruct::data_type type;
            switch (col->type)
            {
            case column_type::Int:
                {
                    type = instruct::data_type::Int;
                    break;
                }
            case column_type::Float:
                {
                    type = instruct::data_type::Float;
                    break;
                }
                case column_type::text:
                {
                    type = instruct::data_type::Text;
                    break;
                }
                default:
                    throw std::runtime_error("unknown column type");
            }
            ins.emplace_back(instruct::op_type::load_col,type,offset);
            ins.emplace_back(instruct::op_type::end,instruct::data_type::Int,0L);
        }
    }

}
