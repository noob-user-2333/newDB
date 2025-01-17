//
// Created by user on 25-1-16.
//
#include "db.h"

namespace iedb
{
    bool db::reader::next()
    {
        static std::stack<expr::item> stack;
        while (iterator->next())
        {
            auto record_size = iterator->get_record_size();
            auto record = iterator->get_record_data();
            //使用where子句对该记录进行判断
            auto offset = 0;
            auto vec_size = where.size();
            auto satisfy = true;
            while (offset < vec_size)
            {
                offset = expr::expr_execute(where, offset, stack, record);
                offset++;
                auto top = stack.top();
                stack.pop();
                if (top.value_int == 0)
                {
                    satisfy = false;
                    break;
                }
                assert(stack.empty());
            }
            if (satisfy)
            {
                //开始提取数据
                offset = 0;
                vec_size = select.size();
                values.clear();
                while (offset < vec_size)
                {
                    offset = expr::expr_execute(select, offset, stack, record);
                    offset++;
                    auto top = stack.top();
                    stack.pop();
                    //开始获取数据
                    switch (top.type)
                    {
                    case expr::item::item_type::Float:
                        {
                            values.emplace_back(top.value_float);
                            break;
                        }
                    case expr::item::item_type::Int:
                        {
                            values.emplace_back(top.value_int);
                            break;
                        }
                    case expr::item::item_type::String:
                        {
                            values.emplace_back(std::string(top.value_string, top.len));
                            break;
                        }
                    default:
                        throw std::runtime_error("unsupported type for select");
                    }
                    assert(stack.empty());
                }
                return true;
            }
        }
        return false;
    }

    int db::reader::get_int_value(uint32 index, int64& out_value) const
    {
        if (index >= values.size())
            return status_out_of_range;
        auto& value = values[index];
        switch (value.index())
        {
        case 0:
            out_value = std::get<int64>(value);
            return status_ok;
        case 1:
            out_value = static_cast<int64>(std::get<double>(value));
            return status_ok;
        case 2:
            return status_error_data_type;
        default:
            throw std::runtime_error("should not run here of get_int_value");
        }
    }

    int db::reader::get_float_value(uint32 index, double& out_value) const
    {
        if (index >= values.size())
            return status_out_of_range;
        auto& value = values[index];
        switch (value.index())
        {
        case 0:
            out_value = static_cast<double>(std::get<int64>(value));
            return status_ok;
        case 1:
            out_value = std::get<double>(value);
            return status_ok;
        case 2:
            return status_error_data_type;
        default:
            throw std::runtime_error("should not run here of get_int_value");
        }
    }

    int db::reader::get_string_value(uint32 index, std::string& value) const
    {
        if (index >= values.size())
            return status_out_of_range;
        auto& v = values[index];
        if (v.index() != 2)
            return status_error_data_type;
        value = std::get<std::string>(v);
        return status_ok;
    }
}

