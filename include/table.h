//
// Created by user on 24-12-17.
//

#ifndef TABLE_H
#define TABLE_H

#include <unordered_map>

#include "utility.h"
/*
 *  Table 核心功能为读取对应文件生成
 *
 *
 */


namespace iedb
{
    enum class column_type
    {
        unknown,
        Int,
        Float,
        text
    };
    using column_data_type = std::variant<int64, double, std::string>;
    struct col_def
    {
        static constexpr int name_len = 32;
        char name[name_len];
        column_type type;
        //该列包含的该类型数据个数
        //非数组则为1
        uint32 element_count;
        //对固定长度数据保存其在行数据中的偏移量
        //对变长数据保存其长度在行数据中的偏移量
        int32 line_offset;

        col_def() = default;
        col_def(const char* name, column_type type, uint32 element_count, int32 line_offset);
    };

    class row
    {
    private:
        const int64 size;
        std::unique_ptr<column_data_type[]> values;

    public:
        row() = delete;
        row(row&) = delete;

        explicit row(const int64 size): size(size),
                                        values(new column_data_type[size], std::default_delete<column_data_type[]>())
        {
        }

        column_data_type& operator[](const int index)
        {
            if (index >= size)
                throw std::out_of_range("row index out of range");
            return values[index];
        }
        [[nodiscard]] int64 get_column_count() const{return size;}
        [[nodiscard]] column_type get_column_type(const int index) const
        {
            if (index >= size)
                throw std::out_of_range("row index out of range");
            switch (values[index].index())
            {
            case 0:
                return column_type::Int;
            case 1:
                return column_type::Float;
            case 2:
                return column_type::text;
            default:
                throw std::runtime_error("unknown column type");
            }
        }
    };

    /*
     *  表定义物理结构
     *   uint32 表名长度 n
     *   uint32 列数 m
     *   n byte 表名
     *   m * 44 列定义
     *
     *
     */
    //核心功能为读取指定文件，将其内容翻译为表格式
    class table
    {
    private:
        std::string name;
        std::unordered_map<std::string,int> name_to_col;
        std::vector<col_def> cols;
        int fixed_len_data_size;

        explicit table(std::string name);
        table(std::string& name, std::vector<col_def>& cols);
        static int get_data_type_size(column_type type);

    public:
        table() = delete;
        static std::unique_ptr<table> get(const void* buffer, uint64 size);
        static std::unique_ptr<table> create_new(std::string name);
        static int64 get_translate_need_buffer_size(table& translate_table);
        static int64 translate_to_buffer(table& translate_table, void* buffer);
        int get_table_size() const;
        const std::string& get_name() const;
        int get_fixed_len_data_size() const;
        int get_col_index_by_name(const std::string& name) const;
        inline const col_def* get_col_by_index(int index) const{return &cols.at(index);}
        int get_col_count() const;
        int add_column(const std::string& name, column_type type, uint32 element_count);

        std::unique_ptr<row> load_row_data_from_record(const void* record, int size) const;
        int load_row_data_from_record(row& row_data,const void* record,int size) const;
    };
}
#endif //TABLE_H
