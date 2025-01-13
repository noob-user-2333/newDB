//
// Created by user on 25-1-3.
//
#include "expr.h"

#include <queue>
#include <stack>
#include <stdexcept>

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
        {token_type::parenthesis_left,100},
        {token_type::plus, 4},
        {token_type::minus, 4},
        {token_type::star, 3},
        {token_type::slash, 3},
        {token_type::percent, 3},
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

    instruct expr::token_to_imm_instruct(const token& node)
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
        default:
            throw std::runtime_error("should never happen for token_to_imm_instruct");
        }
    }

    instruct expr::token_to_col_instruct(const token& token, const col_def& col)
    {
        int64 offset = col.line_offset;
        instruct::data_type type;
        switch (col.type)
        {
        case column_type::Int:
            type = instruct::data_type::Int;
            break;
        case column_type::text:
            type = instruct::data_type::Text;
            break;
        case column_type::Float:
            type = instruct::data_type::Float;
            break;
        default:
            throw std::runtime_error("should never happen for token_to_col_instruct");
        }
        return {instruct::op_type::load_col, type, offset};
    }

    int expr::op_token_process(token_type type, int priority, std::stack<token_type>& stack, std::vector<instruct>& ins)
    {
        stack.push(type);
        return status_ok;
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
}
