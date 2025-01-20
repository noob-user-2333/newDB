//
// Created by user on 25-1-3.
//
#include "expr.h"


namespace iedb
{
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
                return (int)std::get<int64>(new_ins.data);
            ins.push_back(new_ins);
        }
        return status_ok;
    }

    void expr::select_statement_of_star_process(const table& target_table, std::vector<instruct>& ins)
    {
        auto count = target_table.get_col_count();
        for (auto i = 0L; i < count; i++)
        {
            ins.emplace_back(instruct::op_type::load_col, i);
            ins.emplace_back(instruct::get_end_instruct());
        }
    }

    int expr::expr_execute(const std::vector<instruct>& ins, int start_ins_offset, row& row_data,
                           column_data_type& out_result)
    {
        static std::stack<column_data_type> stack;
        auto offset = 0;
        column_data_type item3;
        for (offset = start_ins_offset; ins[offset].op != instruct::op_type::end; offset++)
        {
            auto& current_ins = ins[offset];
            switch (current_ins.op)
            {
            case instruct::op_type::load_col:
                {
                    auto index = static_cast<int>(std::get<int64>(current_ins.data));
                    stack.push(row_data[index]);
                    break;
                }
            case instruct::op_type::load_imm:
                {
                    stack.push(current_ins.data);
                    break;
                }
            case instruct::op_type::add:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_add(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::minus:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_minus(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::mul:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_mul(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::div:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_div(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::mod:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_mod(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::more:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_more(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::less:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_less(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::equal:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_equal(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::more_equal:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_more_equal(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::less_equal:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_less_equal(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            case instruct::op_type::not_equal:
                {
                    const auto item2 = stack.top();
                    stack.pop();
                    const auto item1 = stack.top();
                    stack.pop();
                    auto status = row::column_data_not_equal(item1, item2, item3);
                    if (status != status_ok)
                        return status;
                    stack.push(item3);
                    break;
                }
            default:
                throw std::runtime_error("should never happen for instruct");
            }
        }
        return offset;
    }
}
