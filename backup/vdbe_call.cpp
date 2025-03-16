//
// Created by user on 25-3-6.
//
#include <chrono>

#include "vdbe_call.h"
#include "../include/vdbe_cursor.h"

namespace iedb
{
    static uint64 get_time()
    {

        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
        auto time_us = static_cast<uint64>(duration.count());
        return time_us;
    }
//    static std::string get_stack_string(vdbe &_vdbe)
//    {
//        auto&stack = _vdbe.stack;
//        auto name_len = std::get<int64>(stack.top());
//        stack.pop();
//        auto name_ptr = std::get<void*>(stack.top());
//        stack.pop();
//        return {static_cast<char*>(name_ptr), static_cast<uint64>(name_len)};
//    }
//    static int64 get_stack_num(vdbe&_vdbe)
//    {
//        auto&stack = _vdbe.stack;
//        auto result = std::get<int64>(stack.top());
//        stack.pop();
//        return result;
//    }
//    static void* get_stack_ptr(vdbe&_vdbe)
//    {
//        auto&stack = _vdbe.stack;
//        auto result = std::get<void*>(stack.top());
//        stack.pop();
//        return result;
//    }
//    static double get_stack_double(vdbe& _vdbe)
//    {
//        auto&stack = _vdbe.stack;
//        auto result = std::get<double>(stack.top());
//        stack.pop();
//        return result;
//    }
//
//    int vdbe_call::begin_create_table(vdbe& _vdbe)
//    {
//        //r0,r1 分别对应字符串长度和字符串起始地址
//        auto name_len = std::get<int64>(_vdbe.r[0]);
//        auto name_start = std::get<void*>(_vdbe.r[1]);
//        std::string name(static_cast<const int8*>(name_start),static_cast<size_t>(name_len));
//        _vdbe._table = table::create_new(name);
//        if (_vdbe._table == nullptr)
//            return status_table_exists;
//        return status_ok;
//    }
//
//    int vdbe_call::add_new_column(vdbe& _vdbe)
//    {
//        //r0,r1,r2,r3分别对应字符串长度，字符串起始地址，type和element_count
//        auto name_len = std::get<int64>(_vdbe.r[0]);
//        auto name_start = std::get<void*>(_vdbe.r[1]);
//        auto type = std::get<int64>(_vdbe.r[2]);
//        auto element_count = std::get<int64>(_vdbe.r[3]);
//        return  _vdbe._table->add_column(std::string(static_cast<const int8*>(name_start),static_cast<size_t>(name_len)), static_cast<column_type>(type), element_count);
//    }
//
//    int vdbe_call::create_table(vdbe& _vdbe)
//    {
//        return vdbe_cursor::create_table(*_vdbe._table);
//    }
//    int vdbe_call::open(vdbe& _vdbe)
//    {
//        //获取表名
//        auto name_len = std::get<int64>(_vdbe.r[0]);
//        auto name_start = std::get<void*>(_vdbe.r[1]);
//        std::string name(static_cast<const int8*>(name_start),static_cast<size_t>(name_len));
//        _vdbe.cursor = vdbe_cursor::open(name);
//        if (_vdbe.cursor)
//            return status_ok;
//        return status_not_found;
//    }
//    int vdbe_call::insert(vdbe& _vdbe)
//    {
//        assert(_vdbe.cursor);
//        std::vector<uint8> record;
//        auto key = get_time();
//        //先获取key
//        //获取value并将其格式化到record中
//        vdbe_cursor::row_to_record(_vdbe.row,record);
//        //然后开始插入
//        auto status = _vdbe.cursor->insert(key,record.data(),static_cast<int>(record.size()));
//        _vdbe.row.clear();
//        return status;
//    }
//
//    //TODO:后续处理
//    int vdbe_call::next(vdbe& _vdbe)
//    {
//        auto key_ptr = static_cast<vdbe_value*>(std::get<void*>(_vdbe.r[0]));
//        auto status = _vdbe.cursor->next();
//        if (status == status_ok)
//        {
//            auto key = 0UL;
//            std::vector<uint8> record;
//            _vdbe.cursor->get_record(key,record);
//            //从record中提取数据
//            *key_ptr = static_cast<int64>(key);
//            _vdbe.cursor->record_to_row(record,_vdbe.row);
//        }
//        return status;
//    }

    //  vdbe_call::load_row(vdbe& _vdbe)
    // {
    //     if (_vdbe.it == _vdbe.result.end())
    //     {
    //         _vdbe.status = status_out_of_range;
    //         return;
    //     }
    //     ++_vdbe.it;
    //     _vdbe.row = *_vdbe.it;
    //     _vdbe.status = status_ok;
    // }
    // void vdbe_call::store_row(vdbe& _vdbe)
    // {
    //     _vdbe.result.emplace_back(std::move(_vdbe.row));
    //     _vdbe.status = status_ok;
    // }
}

