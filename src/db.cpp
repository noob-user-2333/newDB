//
// Created by user on 25-1-7.
//
#include "db.h"
#include "expr.h"

#include <stdexcept>

namespace iedb
{
    static column_type token_translate_to_column_type(token* type)
    {
        static std::unordered_map<token_type, column_type> type_map = {
            {token_type::Int, column_type::Int},
            {token_type::Float, column_type::Float},
            {token_type::text, column_type::text}
        };
        return type_map[type->type];
    }

    static column_type token_data_type_to_column_type(token* type)
    {
        static std::unordered_map<token_type, column_type> type_map = {
            {token_type::number_int, column_type::Int},
            {token_type::hex, column_type::Int},
            {token_type::number_float, column_type::Float},
            {token_type::string, column_type::text}
        };
        return type_map[type->type];
    }


    uint64 db::write_statement_to_buffer(const parse_result& result, const table& target_table, void* buffer)
    {
        auto data = static_cast<char*>(buffer);
        auto varLenStart = data + target_table.get_fixed_len_data_size();
        for (auto value = result.master; value; value = value->brother)
        {
            switch (value->type)
            {
            case token_type::number_int:
                {
                    auto int_val = std::stol(value->to_string());
                    memcpy(data, &int_val, sizeof(int_val));
                    data += sizeof(int_val);
                }
                break;
            case token_type::number_float:
                {
                    auto float_val = std::stod(value->to_string());
                    memcpy(data, &float_val, sizeof(float_val));
                    data += sizeof(float_val);
                }
                break;
            case token_type::string:
                {
                    memcpy(data, &(value->len), sizeof(value->len));
                    memcpy(varLenStart, value->sql + value->offset, value->len);
                    data += 8;
                    varLenStart += value->len;
                }
                break;
            case token_type::hex:
                {
                    auto hex_val = std::stoul(value->to_string().c_str() + 2, nullptr, 16);
                    memcpy(data, &hex_val, sizeof(hex_val));
                    data += sizeof(hex_val);
                }
                break;
            default:
                throw std::runtime_error("should not run here of write_statement_to_buffer");
            }
        }
        return ((uint64)varLenStart - (uint64)buffer);
    }

    int db::create_statement_process(parse_result& result)
    {
        assert(result.type == token_type::create);
        auto target_table = data_manager->get_table(result.target->to_string());
        if (target_table)
            return status_table_exists;
        auto new_table = table::create_new(result.target->to_string());
        for (auto col = result.master; col; col = col->brother)
        {
            auto name = col;
            auto type = col->child;
            auto status = new_table->add_column(name->to_string(), token_translate_to_column_type(type), 1);
            if (status != status_ok)
                return status;
        }
        return data_manager->create_table(std::move(new_table));
    }

    int db::insert_statement_process(parse_result& result)
    {
        assert(result.type == token_type::insert);
        static char buffer[1024 * 16];
        auto target_table = data_manager->get_table(result.target->to_string());
        if (!target_table)
            return status_not_find_table;
        //检验数据类型是否正确
        int col_index = 0;
        for (auto col = result.master; col; col = col->brother)
        {
            auto type = token_data_type_to_column_type(col);
            if (type != target_table->get_col_by_index(col_index)->type)
                return status_invalid_data_type;
            col_index++;
        }
        //向buffer中写入数据以便插入
        auto size = write_statement_to_buffer(result, *(target_table), buffer);
        return data_manager->insert_record(target_table->get_name(), buffer, size);
    }

    /*  TODO:当前仅支持where和select语句
     */
    std::unique_ptr<db::reader> db::query_statement_process(parse_result& result)
    {
        //尝试获取对应表
        auto table_name = result.target->to_string();
        auto target_table = data_manager->get_table(table_name);
        if (target_table == nullptr)
        {
            printf("can not found table of %s\n", table_name.c_str());
            return std::unique_ptr<db::reader>(new db::reader{nullptr, status_not_find_table});
        }
        // 构建查询结果
        auto reader = std::unique_ptr<db::reader>(new db::reader(target_table, status_ok));
        //首先解决select为*的特殊情况
        if (result.master->type == token_type::star)
        {
            expr::select_statement_of_star_process(*target_table, reader->select);
        }
        else
        {
            auto status = token_exprs_to_instruction(result.master, *target_table, reader->select);
            if (status != status_ok)
            {
                reader->status_code = status;
                return reader;
            }
        }
        //处理where子句
        auto status = token_exprs_to_instruction(result.filter, *target_table, reader->where);
        if (status != status_ok)
        {
            reader->status_code = status;
            return reader;
        }
        return reader;
    }

    int db::token_expr_to_instruction(token* root, const table& target_table, std::vector<instruct>& ins)
    {
        auto new_root = expr::convert_infix_to_suffix(root);
        return expr::convert_expr_to_instruct(new_root, target_table, ins);
    }

    int db::token_exprs_to_instruction(token* root, const table& target_table, std::vector<instruct>& ins)
    {
        //用于暂存根节点
        static std::queue<token*> queue;
        //暂存根节点
        for (auto node = root; node; node = node->brother)
            queue.push(node);
        //开始将表达式转化为指令
        while (queue.empty() == false)
        {
            auto node = queue.front();
            queue.pop();
            auto status = token_expr_to_instruction(node, target_table, ins);
            if (status != status_ok)
            {
                return status;
            }
            //每个表达式指令结尾添加end
            ins.push_back(instruct::get_end_instruct());
        }
        return status_ok;
    }

    std::unique_ptr<db> db::open(const char* path)
    {
        auto data = db_data_manager::open(path);
        return std::unique_ptr<db>(new db(data));
    }

    int db::sql_execute_without_reader(const char* sql)
    {
        auto result = parse(sql);
        if (result == nullptr)
        {
            fprintf(stderr, "错误的SQL语句:%s\n", sql);
            return status_invalid_sql;
        }
        switch (result->type)
        {
        case token_type::create:
            return create_statement_process(*result);
        case token_type::insert:
            return insert_statement_process(*result);
        // case token_type::select:
        default:
            return status_error;
        }
    }

    std::unique_ptr<db::reader> db::sql_execute(const char* sql)
    {
        auto result = parse(sql);
        if (result == nullptr || result->type != token_type::select)
        {
            fprintf(stderr, "错误的SQL语句:%s\n", sql);
            return std::unique_ptr<reader>(new reader{nullptr, status_invalid_sql});
        }
        return query_statement_process(*result);
    }
}

