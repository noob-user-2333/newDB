//
// Created by user on 25-1-7.
//

#ifndef DB_H
#define DB_H

#include "db_data_manager.h"
#include "expr.h"
#include "parser.h"

namespace iedb
{
    class db
    {
    public:
        class reader
        {
        private:
            using col_data_type =  std::variant<int64,double,std::string>;
            std::vector<col_data_type> values;
        public:
            std::vector<expr::instruct> select;
            std::vector<expr::instruct> where;
            std::vector<expr::instruct> order_by;
            std::vector<expr::instruct> group;
            std::vector<expr::instruct> limit;
            const table* target_table;
            std::unique_ptr<record_iterator> iterator;
            int status_code;
            reader(const table* target_table,int status):target_table(target_table),status_code(status){}
            bool next();
            int get_int_value(uint32 index,int64& out_value) const;
            int get_float_value(uint32 index,double& out_value) const;
            int get_string_value(uint32 index,std::string&value) const;
        };

    private:
        struct string_meta
        {
            uint32 size;
            uint32 offset;
            string_meta(uint32 size,uint32 offset):size(size),offset(offset){}
        };
        std::unique_ptr<db_data_manager> data_manager;
        explicit db(std::unique_ptr<db_data_manager>& data_manager) : data_manager(std::move(data_manager)) {}
        static uint64 write_statement_to_buffer(const parse_result & result,const table&target_table,void * buffer);
        int create_statement_process(parse_result & result);
        int insert_statement_process(parse_result & result);
        std::unique_ptr<reader> query_statement_process(parse_result & result);
        static int token_expr_to_instruction(token* root,const table&target_table,std::vector<expr::instruct>&ins);
        static int token_exprs_to_instruction(token* root,const table&target_table,std::vector<expr::instruct>&ins);
    public:

        static std::unique_ptr<db> open(const char *path);
        int sql_execute_without_reader(const char * sql);
        std::unique_ptr<reader> sql_execute(const char *sql);
    };


}


#endif //DB_H
