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
        struct reader
        {
            std::vector<instruct> select;
            std::vector<instruct> where;
            std::vector<instruct> order_by;
            std::vector<instruct> group;
            std::vector<instruct> limit;
            const table* target_table;
            int status_code;
            reader(const table* target_table,int status):target_table(target_table),status_code(status){}
        };

    private:
        std::unique_ptr<db_data_manager> data_manager;
        explicit db(std::unique_ptr<db_data_manager>& data_manager) : data_manager(std::move(data_manager)) {}
        static uint64 write_statement_to_buffer(const parse_result & result,const table&target_table,void * buffer);
        int create_statement_process(parse_result & result);
        int insert_statement_process(parse_result & result);
        std::unique_ptr<reader> query_statement_process(parse_result & result);
        static int token_expr_to_instruction(token* root,const table&target_table,std::vector<instruct>&ins);
        static int token_exprs_to_instruction(token* root,const table&target_table,std::vector<instruct>&ins);
    public:

        static std::unique_ptr<db> open(const char *path);
        int sql_execute_without_reader(const char * sql);
        std::unique_ptr<reader> sql_execute(const char *sql);
    };


}


#endif //DB_H
