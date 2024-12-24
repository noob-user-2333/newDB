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
        switch (type) {
            case column_type::int8:
            case column_type::uint8:
                return 1;
            case column_type::int16:
            case column_type::uint16:
                return 2;
            case column_type::int32:
            case column_type::uint32:
            case column_type::blob:
                return 4;
            case column_type::int64:
            case column_type::uint64:
            case column_type::float_:
                return 8;
            default:
                throw std::invalid_argument("未知column_type");
        }
    }
    col_def::col_def(const char *name, column_type type, uint32 element_count, int32 line_offset):name(),type(type),element_count(element_count),line_offset(line_offset) {
        auto len = strlen(name);
        if (len > col_def::name_len)
            len = col_def::name_len;
        std::memcpy(this->name, name, len);
    }

    int table::format_parse(void *table_format, uint64 size, std::string &out_name, std::vector<col_def> &out_cols) {
        auto format = static_cast<const uint8 *>(table_format);
        auto offset = 0;
        out_name = std::string(format, format + 256);
        offset += 256;
        uint32 col_count = *(reinterpret_cast<const uint32 *>(format + offset));
        offset += sizeof(col_count);
        out_cols.resize(col_count);
        for (int i = 0; i < col_count; ++i) {
            if (offset >= size)
                return status_error;
            std::memcpy(&out_cols[i], format + offset, sizeof(col_def));
            offset += sizeof(col_def);
        }
        return status_ok;
    }

    table::table(std::string &name, std::vector<col_def> &cols) : name(std::move(name)), cols(std::move(cols)),
                                                                  name_to_col(),fixed_len_data_size() {
        for (auto &col: cols) {
            name_to_col[col.name] = &col;
        }
        fixed_len_data_size = cols.back().line_offset + get_data_type_size(cols.back().type) * cols.back().element_count;
    }
    table::table(std::string name):name(std::move(name)), name_to_col(),cols(),fixed_len_data_size(0){
    }


    // std::unique_ptr<table> table::create_from_file(int fd) {
    //     uint64 size;
    //     auto status = os::get_file_size(fd, size);
    //     if (status != status_ok)
    //         return nullptr;
    //     uint8 buffer[size];
    //     //读取文件
    //     status = os::read(fd, 0, buffer, size);
    //     if (status != status_ok) {
    //         fprintf(stderr, "read file of table format\n");
    //         return nullptr;
    //     }
    //     os::close(fd);
    //     std::string name;
    //     std::vector<col_def> cols;
    //     status = format_parse(buffer, size, name, cols);
    //     if (status == status_ok)
    //         return std::unique_ptr<table>(new table(name, cols));
    //     return nullptr;
    // }
    std::unique_ptr<table> table::create_new(const char * name) {
        return std::unique_ptr<table>(new table(name));
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
    const col_def *table::get_col_by_name(const std::string &name) {
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
        auto& last_col = cols.back();
        auto offset = last_col.line_offset + last_col.element_count * get_data_type_size(last_col.type);
        cols.emplace_back(name.c_str(), type, element_count,offset);
        name_to_col[name] = &cols.back();
        fixed_len_data_size += static_cast<int>(element_count) * get_data_type_size(type);
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

