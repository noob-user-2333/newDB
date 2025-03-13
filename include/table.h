//
// Created by user on 24-12-17.
//

#ifndef TABLE_H
#define TABLE_H

#include <unordered_map>

#include "token.h"
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
    using column_value = std::variant<uint64, double, std::string>;
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
        static int64 translate_to_buffer(const table& translate_table,std::vector<uint8> &buffer);
        static column_type translate_token_to_column_type(token_type type);
        int get_table_size() const;
        const std::string& get_name() const;
        int get_fixed_len_data_size() const;
        int get_col_index_by_name(const std::string& name) const;
        inline const col_def* get_col_by_index(int index) const{return &cols.at(index);}
        int get_col_count() const;
        int add_column(const std::string& name, column_type type, uint32 element_count);

        void load_row_from_record(const std::vector<uint8> & record,std::vector<column_value>&row) const;
        int store_row_to_record(const std::vector<column_value>& row_data,std::vector<uint8> & out_record) const;
    };
}
#endif //TABLE_H
