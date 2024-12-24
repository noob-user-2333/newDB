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



namespace iedb {
    enum class column_type {
        unknown,
        int8,
        int16,
        int32,
        int64,
        uint8,
        uint16,
        uint32,
        uint64,
        float_,
        blob
    };

    struct col_def {
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
        col_def(const char *name,column_type type,uint32 element_count, int32 line_offset);
    };


    //核心功能为读取指定文件，将其内容翻译为表格式
    class table {
    private:
        std::string name;
        std::unordered_map<std::string,col_def*> name_to_col;
        std::vector<col_def> cols;
        int fixed_len_data_size;

        table() = delete;
        explicit table(std::string name);
        table(std::string& name,std::vector<col_def>& cols);
        static int format_parse(void *table_format,uint64 size,std::string &out_name,std::vector<col_def> &out_cols);
        static int get_data_type_size(column_type type);
    public:
        // static std::unique_ptr<table> create_from_file(int fd);
        static std::unique_ptr<table> create_new(const char * name);
        int get_table_size() const;
        int get_fixed_len_data_size() const;
        const col_def* get_col_by_name(const std::string& name);
        const col_def* get_col_by_index(int index) const;
        int get_col_count() const;
        int add_column(const std::string& name, column_type type, uint32 element_count);
        // int remove_column(const std::string& name);
        // int update_column(const std::string& name,const std::string&new_name,column_type new_type, uint32 new_element_count);

    };




}
#endif //TABLE_H
