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
    struct string_meta {
        uint32 offset;
        uint32 size;
    };

    int table::get_data_type_size(column_type type) {
        return 8;
    }

    col_def::col_def(const char *name, column_type type, uint32 element_count, int32 line_offset): name(), type(type),
        element_count(element_count), line_offset(line_offset) {
        auto len = strlen(name);
        if (len > col_def::name_len)
            len = col_def::name_len;
        std::memcpy(this->name, name, len);
    }


    table::table(std::string &name, std::vector<col_def> &cols) : name(std::move(name)), cols(std::move(cols)),
                                                                  name_to_col(), fixed_len_data_size() {
        auto i = 0;
        for (auto &col: this->cols) {
            name_to_col[col.name] = i;
            i++;
        }
        fixed_len_data_size = this->cols.back().line_offset + get_data_type_size(this->cols.back().type) * this->cols.
                              back().element_count;
    }

    table::table(std::string name): name(std::move(name)), name_to_col(), cols(), fixed_len_data_size(0) {
    }

    const std::string &table::get_name() const {
        return name;
    }


    std::unique_ptr<table> table::get(const void *buffer, uint64 size) {
        auto data = static_cast<const char *>(buffer);
        auto name_len = *(uint32 *) data;
        auto col_count = *(uint32 *) (data + 4);
        //检查size是否正确
        if (size != col_count * sizeof(col_def) + 8 + name_len)
            return nullptr;
        data += 8;
        std::string name(data, data + name_len);
        data += name_len;
        std::vector<col_def> cols;
        cols.resize(col_count);
        //开始生成对应列
        for (auto i = 0; i < col_count; i++) {
            memcpy(&cols[i], data, sizeof(col_def));
            data += sizeof(col_def);
        }
        return std::unique_ptr<table>(new table(name, cols));
    }

    std::unique_ptr<table> table::create_new(std::string name) {
        return std::unique_ptr<table>(new table(std::move(name)));
    }

    int64 table::translate_to_buffer(const table &translate_table,std::vector<uint8>&buffer) {
        auto size = 8 + translate_table.name.size() + translate_table.cols.size() * sizeof(col_def);
        buffer.resize(size);
        auto data = reinterpret_cast<char *>(buffer.data());
        *(uint32 *) data = translate_table.name.size();
        data += 4;
        auto col_count = translate_table.cols.size();
        *(uint32 *) data = col_count;
        data += 4;
        memcpy(data, translate_table.name.c_str(), translate_table.name.size());
        data += translate_table.name.size();
        for (auto i = 0; i < col_count; i++) {
            memcpy(data, &translate_table.cols[i], sizeof(col_def));
            data += sizeof(col_def);
        }
        return static_cast<int64>(size);
    }

    column_type table::translate_token_to_column_type(token_type type) {
        switch (type) {
            case token_type::Int:
                return column_type::Int;
            case token_type::Float:
                return column_type::Float;
            case token_type::text:
                return column_type::text;
            default:
                return column_type::unknown;
        }
    }


    int table::get_table_size() const {
        return static_cast<int>(256 + sizeof(uint32) + sizeof(col_def) * cols.size());
    }

    int table::get_fixed_len_data_size() const {
        return fixed_len_data_size;
    }

    int table::get_col_index_by_name(const std::string &name) const {
        auto it = name_to_col.find(name);
        if (it == name_to_col.end())
            return -1;
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
        for (const auto &col: cols) {
            offset += get_data_type_size(col.type) * col.element_count;
        }
        cols.emplace_back(name.c_str(), type, element_count, offset);
        fixed_len_data_size += get_data_type_size(type) * element_count;
        return status_ok;
    }

    // std::unique_ptr<row> table::load_row_data_from_record(const void* record, int size) const
    // {
    //     auto data = static_cast<const char*>(record);
    //     auto col_count = cols.size();
    //     auto row_data = std::make_unique<row>(col_count);
    //     for (auto i = 0; i< col_count;i++)
    //     {
    //         auto& col = cols[i];
    //         auto start = data + col.line_offset;
    //         int64 offset = col.line_offset;
    //         switch (col.type)
    //         {
    //             case column_type::Int:
    //                 {
    //                     (*row_data)[i] = *(int64*)start;
    //                     offset += sizeof(int64);
    //                     break;
    //                 }
    //             case column_type::Float:
    //                 {
    //                     (*row_data)[i] = *(double*)start;
    //                     offset += sizeof(double);
    //                     break;
    //                 }
    //             case column_type::text:
    //                 {
    //                     //先获取长度和偏移量
    //                     auto meta = (uint32*)data;
    //                     auto str_size = meta[0];
    //                     auto str_offset = meta[1];
    //                     (*row_data)[i] = std::string(data + str_offset, str_size);
    //                     offset = str_size + str_offset;
    //                     break;
    //                 }
    //             default:
    //                 throw std::runtime_error("should not happen in table::load_row_data_from_record");
    //         }
    //         assert(offset <= size);
    //     }
    //     return std::move(row_data);
    // }
    //
    void table::load_row_from_record(const std::vector<uint8> &record, std::vector<column_value> &row) const {
        auto col_count = cols.size();
        row.resize(col_count);
        auto data = record.data();
        for (auto i = 0; i < col_count; i++) {
            auto &col = cols[i];
            int64 offset = col.line_offset;
            auto start = data + offset;
            switch (col.type) {
                case column_type::Int: {
                    row[i] = *reinterpret_cast<const int64 *>(start);
                    break;
                }
                case column_type::Float: {
                    row[i] = *reinterpret_cast<const double *>(start);
                    break;
                }
                case column_type::text: {
                    //先获取长度和偏移量
                    auto meta = reinterpret_cast<const string_meta *>(start);
                    row[i] = std::string((char *) (data + meta->offset), meta->size);
                    break;
                }
                default:
                    throw std::runtime_error("should not happen in table::load_row_data_from_record");
            }
        }
    }

    int table::store_row_to_record(const std::vector<column_value> &row_data, std::vector<uint8> &out_record) const {
        //首先确定记录长度
        auto fix_len = 8 * row_data.size();
        auto len_count = fix_len;
        auto row_count = row_data.size();
        for (auto i = 0; i < row_count; i++) {
            const auto &col = cols[i];
            auto index = row_data[i].index();
            switch (col.type) {
                case column_type::Int: {
                    if (index != 0)
                        return status_error_data_type;
                    break;
                }
                case column_type::Float: {
                    if (index != 1)
                        return status_error_data_type;
                    break;
                }
                case column_type::text: {
                    if (index != 2)
                        return status_error_data_type;
                    const std::string &str = std::get<std::string>(row_data[i]);
                    len_count += str.size();
                    break;
                }
                default:
                    throw std::runtime_error("should not happen in table::store_row_to_record");
            }
        }
        out_record.resize(len_count);
        auto row_offset = fix_len;
        for (auto i = 0 ;i < row_count ; i++) {
            const auto &col = cols[i];
            switch (col.type) {
                case column_type::Int: {
                    auto offset = cols[i].line_offset;
                    auto value = std::get<int64>(row_data[i]);
                    memcpy(out_record.data() + offset,&value,sizeof(double));
                    break;
                }
                case column_type::Float: {
                    auto offset = cols[i].line_offset;
                    auto value = std::get<double>(row_data[i]);
                    memcpy(out_record.data() + offset,&value,sizeof(double));
                    break;
                }
                case column_type::text: {
                    auto offset = cols[i].line_offset;
                    auto& value = std::get<std::string>(row_data[i]);
                    auto meta = reinterpret_cast<string_meta *>(out_record.data() + offset);
                    meta->offset = row_offset;
                    meta->size = value.size();
                    memcpy(out_record.data() + row_offset,value.c_str(),value.size());
                    row_offset += value.size();
                    break;
                }
                default:
                    throw std::runtime_error("should not happen in table::store_row_to_record");
            }

        }
        return status_ok;
    }
}



