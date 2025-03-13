//
// Created by user on 25-3-5.
//
#include "vdbe_stmt.h"

#include "vdbe_call.h"

namespace iedb
{
    // token* vdbe_stmt::convert_infix_to_suffix(token* root)
    // {
    //     static const std::unordered_map<token_type, int> op_priority = {
    //         {token_type::parenthesis_left, 100},
    //         {token_type::star, 3},
    //         {token_type::slash, 3},
    //         {token_type::percent, 3},
    //         {token_type::plus, 4},
    //         {token_type::minus, 4},
    //         {token_type::less, 6},
    //         {token_type::less_equal, 6},
    //         {token_type::more, 6},
    //         {token_type::more_equal, 6},
    //         {token_type::equal, 7},
    //         {token_type::not_equal, 7},
    //         {token_type::And, 11},
    //         {token_type::Or, 12}
    //     };
    //     static std::stack<token*> stack;
    //     static std::queue<token*> suffixQueue;
    //     assert(root && stack.empty() && suffixQueue.empty());
    //     auto brother = root->brother;
    //     for (auto node = root; node; node = node->child)
    //     {
    //         switch (node->type)
    //         {
    //         //算数类单词直接入队
    //         case token_type::name:
    //         case token_type::number_float:
    //         case token_type::number_int:
    //         case token_type::hex:
    //         case token_type::string:
    //             {
    //                 suffixQueue.push(node);
    //                 break;
    //             }
    //         //遭遇左括号直接入栈
    //         case token_type::parenthesis_left:
    //             {
    //                 stack.push(node);
    //                 break;
    //             }
    //         //遭遇右括号则一直出栈，直到遇到第一个左括号
    //         case token_type::parenthesis_right:
    //             {
    //                 while (stack.top()->type != token_type::parenthesis_left)
    //                 {
    //                     suffixQueue.push(stack.top());
    //                     stack.pop();
    //                 }
    //                 stack.pop();
    //                 break;
    //             }
    //         //当传入token为运算符时
    //         default:
    //             {
    //                 auto it = op_priority.find(node->type);
    //                 //不应当存在非运算符单词
    //                 if (it == op_priority.end())
    //                 {
    //                     char buff[4096];
    //                     snprintf(buff, sizeof(buff), "invalid token for %s\n", node->to_string().c_str());
    //                     throw std::runtime_error(buff);
    //                 }
    //                 int priority = it->second;
    //                 //开始处理
    //                 //在栈非空时，需确保新插入元素优先级大于栈顶元素
    //                 while (stack.empty() == false)
    //                 {
    //                     auto top = stack.top();
    //                     auto top_priority = op_priority.find(top->type)->second;
    //                     if (priority >= top_priority)
    //                     {
    //                         suffixQueue.push(stack.top());
    //                         stack.pop();
    //                     }
    //                     else
    //                         break;
    //                 }
    //                 stack.push(node);
    //             }
    //         }
    //     }
    //     //将stack中的运算符全部push
    //     while (stack.empty() == false)
    //     {
    //         suffixQueue.push(stack.top());
    //         stack.pop();
    //     }
    //     //将suffixQueue中的token全部pop到root
    //     auto new_root = suffixQueue.front();
    //     auto current = suffixQueue.front();
    //     suffixQueue.pop();
    //     while (suffixQueue.empty() == false)
    //     {
    //         current->child = suffixQueue.front();
    //         current = current->child;
    //         suffixQueue.pop();
    //     }
    //     current->child = nullptr;
    //     new_root->brother = brother;
    //     return new_root;
    // }
    //
    // void vdbe_stmt::convert_token_to_expr_op(std::vector<uint8>& ins, token* token_op)
    // {
    //
    //     switch (token_op->type)
    //     {
    //     case token_type::string:
    //         {
    //             ins.emplace_back(static_cast<uint8>(vdbe_op::load_imm_num));
    //
    //         }
    //         case token_type::number_int:
    //             {
    //                 char* end;
    //                 auto num = std::strtol(token_op->sql + token_op->offset, &end, 10);
    //                 assert(end == token_op->sql + token_op->offset + token_op->len);
    //                 return {vdbe_op::store_int, 0, 0, static_cast<uint64>(num)};
    //             }
    //         case token_type::number_float:
    //             {
    //                 char* end;
    //                 auto num = std::strtod(token_op->sql + token_op->offset, &end);
    //                 assert(end == token_op->sql + token_op->offset + token_op->len);
    //                 return {vdbe_op::store_double, 0, 0, num};
    //             }
    //         case token_type::hex:
    //         {
    //             char* end;
    //             auto num = std::strtol(token_op->sql + token_op->offset + 2, &end, 16);
    //             assert(end == token_op->sql + token_op->offset + token_op->len);
    //             return {vdbe_op::store_int, 0, 0, static_cast<uint64>(num)};
    //         }
    //         //TODO:记得处理列名的情况
    //     case token_type::plus:
    //         return {vdbe_op::add, 0, 0, nullptr};
    //     case token_type::minus:
    //         return {vdbe_op::minus,0,0,0UL};
    //     case token_type::star:
    //         return {vdbe_op::mul, 0, 0, nullptr};
    //     case token_type::slash:
    //             return {vdbe_op::div, 0, 0, nullptr};
    //     case token_type::percent:
    //         return {vdbe_op::mod, 0, 0, nullptr};
    //     case token_type::less:
    //         return {vdbe_op::less, 0, 0, nullptr};
    //     case token_type::less_equal:
    //         return {vdbe_op::less_equal, 0, 0, nullptr};
    //     case token_type::more:
    //         return {vdbe_op::more, 0, 0, nullptr};
    //     case token_type::more_equal:
    //         return {vdbe_op::more_equal, 0, 0, nullptr};
    //     case token_type::equal:
    //         return {vdbe_op::equal, 0, 0, nullptr};
    //     case token_type::not_equal:
    //         return {vdbe_op::not_equal, 0, 0, nullptr};
    //     case token_type::And:
    //         return {vdbe_op::And, 0, 0, nullptr};
    //     case token_type::Or:
    //         return {vdbe_op::Or, 0, 0, nullptr};
    //     default:
    //         throw std::runtime_error("unsupported token type");
    //     }
    // }
    //
    // void vdbe_stmt::append_call(std::vector<uint8>& ins, vdbe_func_ptr func)
    // {
    //     uint8 buffer[8];
    //     memcpy(buffer,&func,sizeof(func));
    //     ins.emplace_back(static_cast<uint8>(vdbe_op::call));
    //     ins.insert(ins.end(),buffer,buffer + sizeof(buffer));
    // }
    // void vdbe_stmt::append_push(std::vector<uint8>& ins, int r_index)
    // {
    //     ins.emplace_back(static_cast<uint8>(vdbe_op::push));
    //     ins.emplace_back(static_cast<uint8>(r_index));
    // }
    // void vdbe_stmt::append_new_col(std::vector<uint8>& ins, int64 name_len, void* name_start, int64 type, int64 element_count)
    // {
    //     append_load_imm(ins,3,element_count);
    //     append_load_imm(ins,2,type);
    //     append_load_imm(ins,1,name_start);
    //     append_load_imm(ins,0,name_len);
    // }
    // void vdbe_stmt::append_load_imm(std::vector<uint8>& ins, int r_index, double decimal)
    // {
    //     ins.emplace_back(static_cast<uint8>(vdbe_op::load_imm_decimal));
    //     ins.emplace_back(static_cast<uint8>(r_index));
    //     uint8 buffer[8];
    //     memcpy(buffer,&decimal,sizeof(decimal));
    //     ins.insert(ins.end(),buffer,buffer + sizeof(buffer));
    // }
    // void vdbe_stmt::append_load_imm(std::vector<uint8>& ins, int r_index, int64 num)
    // {
    //     ins.emplace_back(static_cast<uint8>(vdbe_op::load_imm_num));
    //     ins.emplace_back(static_cast<uint8>(r_index));
    //     uint8 buffer[8];
    //     memcpy(buffer,&num,sizeof(num));
    //     ins.insert(ins.end(),buffer,buffer + sizeof(buffer));
    // }
    // void vdbe_stmt::append_load_imm(std::vector<uint8>& ins, int r_index, void* ptr)
    // {
    //     ins.emplace_back(static_cast<uint8>(vdbe_op::load_imm_ptr));
    //     ins.emplace_back(static_cast<uint8>(r_index));
    //     uint8 buffer[8];
    //     memcpy(buffer,&ptr,sizeof(ptr));
    //     ins.insert(ins.end(),buffer,buffer + sizeof(buffer));
    //
    // }




    // std::unique_ptr<vdbe_stmt> vdbe_stmt::prepare_create(std::unique_ptr<parse_result>& result)
    // {
    //     assert(result->type == token_type::create);
    //     std::vector<uint8> ins;
    //     std::vector<token*> cols;
    //     auto name_node = result->target;
    //     append_load_imm(ins,0,static_cast<int64>(ins.size()));
    //     append_load_imm(ins,1,static_cast<int64>(name_node->len));
    //     append_load_imm(ins,2,(void*)(name_node->sql + name_node->offset));
    //     append_call(ins,vdbe_call::begin_create_table);
    //     for (auto col = result->master; col; col = col->brother){
    //         name_node = col;
    //         auto type_node = col->child;
    //         auto type = table::translate_token_to_column_type(type_node->type);
    //         auto element_count = 1;
    //         append_new_col(ins,name_node->len,(void*)(name_node->sql + name_node->offset),static_cast<int64>(type),element_count);
    //     }
    //     append_call(ins,vdbe_call::create_table);
    //     return std::unique_ptr<vdbe_stmt>(new vdbe_stmt(result, ins));
    // }
    //
    //
    // std::unique_ptr<vdbe_stmt> vdbe_stmt::prepare_insert(std::unique_ptr<parse_result>& result)
    // {
    //     assert(result->type == token_type::insert);
    //     std::vector<uint8> ins;
    //     //打开对应数据库表
    //     auto name_node = result->target;
    //     append_load_imm(ins,0,static_cast<int64>(name_node->len));
    //     append_load_imm(ins,1,(void*)(name_node->sql + name_node->offset));
    //     append_call(ins,vdbe_call::open);
    //     //装载数据
    //     for (auto col = result->master; col; col = col->brother)
    //     {
    //         switch (col->type)
    //         {
    //         case token_type::number_float:
    //             {
    //                 uint8 buffer[9];
    //                 char* end;
    //                 auto num = std::strtod(col->sql + col->offset, &end);
    //                 assert(end == col->sql + col->offset + col->len);
    //                 buffer[0] = static_cast<uint8>(vdbe_op::store_double);
    //                 memcpy(buffer+1,&num,sizeof(num));
    //                 ins.insert(ins.end(),buffer,buffer+sizeof(buffer));
    //                 break;
    //             }
    //         case token_type::number_int:
    //             {
    //                 uint8 buffer[9];
    //                 char* end;
    //                 auto num = std::strtol(col->sql + col->offset, &end, 10);
    //                 assert(end == col->sql + col->offset + col->len);
    //                 buffer[0] = static_cast<uint8>(vdbe_op::store_int);
    //                 memcpy(buffer+1,&num,sizeof(num));
    //                 ins.insert(ins.end(),buffer,buffer+sizeof(buffer));
    //                 break;
    //             }
    //         case token_type::hex:
    //             {
    //                 uint8 buffer[9];
    //                 char* end;
    //                 auto num = std::strtol(col->sql + col->offset + 2, &end, 16);
    //                 assert(end == col->sql + col->offset + col->len);
    //                 buffer[0] = static_cast<uint8>(vdbe_op::store_int);
    //                 memcpy(buffer+1,&num,sizeof(num));
    //                 ins.insert(ins.end(),buffer,buffer+sizeof(buffer));
    //                 break;
    //             }
    //         case token_type::string:
    //             {
    //                 auto len_start = reinterpret_cast<const uint8*>(&col->len);
    //                 auto len_end = len_start + sizeof(uint32);
    //                 auto str_start = col->sql + col->offset;
    //                 ins.emplace_back(static_cast<uint8>(vdbe_op::store_string));
    //                 ins.insert(ins.end(),len_start,len_end);
    //                 ins.insert(ins.end(),str_start,str_start + col->len);
    //                 break;
    //             }
    //         default:
    //                 throw std::runtime_error("should not happen");
    //         }
    //     }
    //     //调用函数插入数据
    //     append_call(ins,vdbe_call::insert);
    //     return std::unique_ptr<vdbe_stmt>(new vdbe_stmt(result, ins));
    // }
    //
    // std::unique_ptr<vdbe_stmt> vdbe_stmt::prepare_select(std::unique_ptr<parse_result>& result)
    // {
    //     assert(result->type == token_type::select);
    //     // std::vector<vdbe_op> ins;
    //     // //打开指定表
    //     // ins.emplace_back(vdbe_op::load_imm_string, 0, result->target->len,
    //     //                  (void*)(result->target->sql + result->target->offset));
    //     // ins.emplace_back(vdbe_op::call, 0, 0, reinterpret_cast<void*>(vdbe_call::open));
    //     // ins.emplace_back(vdbe_op::load_status, 0, 0, 0UL);
    //     // ins.emplace_back(vdbe_op::jz, 1, 0, 0UL);
    //     // ins.emplace_back(vdbe_op::error, 0, 0, (void*)"can not open table");
    //     // /*
    //     // *  sql查询语句在不包含子查询与join的情况下，可大致翻译为以下代码
    //     //     while(cursor.next == status_ok){
    //     //         if(where_statement){
    //     //             result.push_back(row);
    //     //         }
    //     //     }
    //     //     sort(result);
    //     //     foreach(row_data:result){
    //     //         foreach(select : select_statement){
    //     //             select(row_data);
    //     //         }
    //     //     }
    //     //  */
    //     // //调用next，并根据状态码判断是否进行下一步
    //     // ins.emplace_back(vdbe_op::call, 0, 0, reinterpret_cast<void*>(vdbe_call::next));
    //     // ins.emplace_back(vdbe_op::load_status, 0, 0, 0UL);
    //     // //如果正常读取数据则进入where筛选指令
    //     // ins.emplace_back(vdbe_op::jz, 4, 0, 0UL);
    //     // ins.emplace_back(vdbe_op::load_status, 0, 0, 0UL);
    //     // ins.emplace_back(vdbe_op::equal, 0, 0, static_cast<uint64>(status_out_of_range));
    //     // //判断是否为数据读取完毕，如果读取完毕需跳出where筛选指令
    //     // //暂存该指令索引，确定筛选指令数量后更改跳转数
    //     // auto jump_out_where_index = ins.size();
    //     // ins.emplace_back(vdbe_op::jnz, 0, 0, 0UL);
    //     // ins.emplace_back(vdbe_op::error, 0, 0, (void*)"can not get record");
    //     // //根据where_statement对结果进行过滤
    //     // if (result->filter)
    //     // {
    //     //
    //     // }
    //     //
    //
    //     return nullptr;
    // }
    //
    //
    // std::unique_ptr<vdbe_stmt> vdbe_stmt::prepare(std::unique_ptr<parse_result>& result)
    // {
    //     switch (result->type)
    //     {
    //     case token_type::select:
    //         return prepare_select(result);
    //     case token_type::create:
    //         return prepare_create(result);
    //     case token_type::insert:
    //         return prepare_insert(result);
    //     default:
    //         fprintf(stderr, "未知的sql查询语句");
    //         return nullptr;
    //     }
    // }
    //
    // std::unique_ptr<vdbe_stmt> vdbe_stmt::prepare(const char* sql)
    // {
    //     auto result = parse(sql);
    //     return prepare(result);
    // }

    // int vdbe_stmt::execute(vdbe_stmt& stmt)
    // {
    //     vdbe _vdbe;
    //     auto size = stmt.ins.size();
    //     while (_vdbe.pc < size)
    //     {
    //         auto status = _vdbe.execute_instruct(stmt.ins);
    //         if (status != status_ok)
    //         {
    //             fprintf(stderr, "something wrong when vdbe run call\n");
    //             return status;
    //         }
    //     }
    //     return status_ok;
    // }


    // vdbe_stmt::vdbe_stmt(std::unique_ptr<parse_result>& result, std::vector<uint8>& ins): result(std::move(result)),
        // ins(std::move(ins))
    // {
    // }
}


