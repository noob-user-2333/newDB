//
// Created by user on 24-12-17.
//
#include "table.h"
#include <stdexcept>
/*
 *
 *  struct table_format {
        char name[256];
        uint32 col_count;
        column columns[col_count]
    };
    struct column{
        char name[32];
        uint32 element_count;
        uint32 type;
    };
 */

namespace iedb {
    int table::get_data_type_size(column_type type) {
        return 8;
    }
    col_def::col_def(const char *name, column_type type, uint32 element_count, int32 line_offset):name(),type(type),element_count(element_count),line_offset(line_offset) {
        auto len = strlen(name);
        if (len > col_def::name_len)
            len = col_def::name_len;
        std::memcpy(this->name, name, len);
    }



    table::table(std::string &name, std::vector<col_def> &cols) : name(std::move(name)), cols(std::move(cols)),
                                                                  name_to_col(),fixed_len_data_size() {
        for (auto &col: this->cols) {
            name_to_col[col.name] = &col;
        }
        fixed_len_data_size = this->cols.back().line_offset + get_data_type_size(this->cols.back().type) * this->cols.back().element_count;
    }
    table::table(std::string name):name(std::move(name)), name_to_col(),cols(),fixed_len_data_size(0){
    }
    const std::string &table::get_name() const {
        return name;
    }




    std::unique_ptr<table> table::get(const void* buffer, uint64 size)
    {
        auto data = static_cast<const char*>(buffer);
        auto name_len = *(uint32*)data;
        auto col_count = *(uint32*)(data + 4);
        //检查size是否正确
        if (size != col_count * sizeof(col_def) + 8 + name_len)
            return nullptr;
        data += 8;
        std::string name(data, data + name_len);
        data += name_len;
        std::vector<col_def> cols;
        cols.resize(col_count);
        //开始生成对应列
        for (auto i = 0 ;i < col_count;i++)
        {
            memcpy(&cols[i],data,sizeof(col_def));
            data += sizeof(col_def);
        }
        return std::unique_ptr<table>(new table(name,cols));
    }

    std::unique_ptr<table> table::create_new(std::string name) {
        return std::unique_ptr<table>(new table(std::move(name)));
    }
    int64 table::get_translate_need_buffer_size(table& translate_table)
    {
        int need_size = 8;
        need_size += translate_table.name.size();
        need_size += translate_table.cols.size() * sizeof(col_def);
        return need_size;
    }
    int64 table::translate_to_buffer(table& translate_table, void* buffer)
    {
        auto size = get_translate_need_buffer_size(translate_table);
        auto data = static_cast<char*>(buffer);
        *(uint32*)data = translate_table.name.size();
        data += 4;
        auto col_count = translate_table.cols.size();
        *(uint32*)data = col_count;
        data += 4;
        memcpy(data, translate_table.name.c_str(), translate_table.name.size());
        data += translate_table.name.size();
        for (auto i = 0;i < col_count;i++)
        {
            memcpy(data,&translate_table.cols[i],sizeof(col_def));
            data += sizeof(col_def);
        }
        return size;
    }



    int table::get_table_size() const {
        return static_cast<int>(256 + sizeof(uint32) + sizeof(col_def) * cols.size());
    }
    int table::get_fixed_len_data_size() const {
        return fixed_len_data_size;
    }

    const col_def *table::get_col_by_index(int index) const {
        return &cols.at(index);
    }
    const col_def *table::get_col_by_name(const std::string &name)const  {
        auto it = name_to_col.find(name);
        if (it == name_to_col.end())
            return nullptr;
        return it->second;
    }
    int table::get_col_count() const {
        return static_cast<int>(cols.size());
    }

    int table::add_column(const std::string &name, column_type type, uint32 element_count) {
        auto it = name_to_col.find(name);
        if (it != name_to_col.end())
            return status_column_exists;
        /*
         *  由于所有数据类型均为8字节
         */
        auto offset = 0L;
        for (const auto &col : cols) {
            offset += get_data_type_size(col.type) * col.element_count;
        }
        cols.emplace_back(name.c_str(), type, element_count, offset);
        fixed_len_data_size += get_data_type_size(type) * element_count;
        return status_ok;
    }
    // int table::remove_column(const std::string &name) {
    //     auto it = name_to_col.find(name);
    //     if (it == name_to_col.end())
    //         return status_not_find_column;
    //
    //
    // }

}

