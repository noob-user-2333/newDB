//
// Created by user on 25-1-7.
//

#ifndef DB_H
#define DB_H
#include "db_data_manager.h"
#include "parser.h"

namespace iedb
{
    class db
    {
    private:
        std::unique_ptr<db_data_manager> data_manager;
        explicit db(std::unique_ptr<db_data_manager>& data_manager) : data_manager(std::move(data_manager)) {}

        static uint64 write_statement_to_buffer(const parse_result & result,const table&target_table,void * buffer);
        int create_statement_process(parse_result & result);
        int insert_statement_process(parse_result & result);
        int query_statement_process(parse_result & result);
    public:
        struct reader
        {
            int status_code;
            explicit reader(int status):status_code(status){}
        };
        static std::unique_ptr<db> open(const char *path);
        int sql_execute_without_reader(const char * sql);
        reader sql_execute(const char *sql);
    };


}


#endif //DB_H
