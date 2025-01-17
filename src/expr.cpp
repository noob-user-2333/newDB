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

    expr::item expr::extract_item_from_record(const instruct & ins,const void* record_data,int offset)
    {
        auto type = ins.op;
        auto data = static_cast<const char*>(record_data);
        switch (type)
        {
        case instruct::op_type::load_col_int:
            {
                auto value = *(int64*)(data + offset);
                return {item::item_type::Int,value};
            }
        case instruct::op_type::load_col_float:
            {
                auto value = *(double*)(data + offset);
                return {item::item_type::Float,value};
            }
        case instruct::op_type::load_col_string:
            {
                auto meta = (uint32*)(data + offset);
                auto size = meta[0];
                auto value_offset = meta[1];
                return {item::item_type::String,size,data + value_offset};
            }
        default:
            {
                throw std::runtime_error("Unsupported instruct type");
            }
        }



    }

    expr::item expr::extract_item_from_imm_instruct(const instruct& ins)
    {
        switch (ins.op)
        {
        case instruct::op_type::load_imm_int:
            {
                return {item::item_type::Int,ins.value_int};
            }
        case instruct::op_type::load_imm_float:
            {
                return {item::item_type::Float,ins.value_float};
            }
        default:
            {
                throw std::runtime_error("Unsupported instruct type");
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
        return status_ok;
    }

    void expr::select_statement_of_star_process(const table& target_table, std::vector<instruct>& ins)
    {
        auto count = target_table.get_col_count();
        for (auto i = 0; i < count;i++)
        {
            auto col = target_table.get_col_by_index(i);
            auto offset = static_cast<int64>(col->line_offset);
            instruct::op_type type;
            switch (col->type)
            {
            case column_type::Int:
                {
                    type = instruct::op_type::load_col_int;
                    break;
                }
            case column_type::Float:
                {
                    type = instruct::op_type::load_col_float;
                    break;
                }
                case column_type::text:
                {
                    type = instruct::op_type::load_col_string;
                    break;
                }
                default:
                    throw std::runtime_error("unknown column type");
            }
            ins.emplace_back(type,offset);
            ins.emplace_back(instruct::get_end_instruct());
        }
    }
    int expr::expr_execute(const std::vector<instruct>& ins, int start_ins_offset, std::stack<item>& stack, const void* record_data)
    {
        auto data = static_cast<const char*>(record_data);
        auto offset = 0;
        for (offset = start_ins_offset;ins[offset].op!= instruct::op_type::end; offset++)
        {
            auto& current_ins = ins[offset];
            switch (current_ins.op)
            {
                case instruct::op_type::load_col_int:
                case instruct::op_type::load_col_float:
                case instruct::op_type::load_col_string:
                    {
                        auto item = extract_item_from_record(current_ins, record_data, static_cast<int>(current_ins.value_int));
                        stack.push(item);
                        break;
                    }
                case instruct::op_type::load_imm_int:
                case instruct::op_type::load_imm_float:
                    {
                        auto item = extract_item_from_imm_instruct(current_ins);
                        stack.push(item);
                        break;
                    }
                case instruct::op_type::add:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 + item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::sub:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 - item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::mul:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 * item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::div:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        if (item2.value_float == 0.0)
                            throw std::runtime_error("division by zero");
                        const auto item3 = item1 / item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::mod:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 % item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::more:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 > item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::less:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 < item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::equal:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 == item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::more_equal:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 >= item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::less_equal:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1 <= item2;
                        stack.push(item3);
                        break;
                    }
                case instruct::op_type::not_equal:
                    {
                        const auto item2 = stack.top();
                        stack.pop();
                        const auto item1 = stack.top();
                        stack.pop();
                        const auto item3 = item1!= item2;
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
