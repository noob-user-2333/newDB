//
// Created by user on 25-1-5.
//

#ifndef DB_DATA_MANAGER_H
#define DB_DATA_MANAGER_H
#include "../third-part/sqlite/sqlite3.h"
#include "utility.h"
#include "table.h"
namespace iedb {
    class record_iterator {
    private:
        std::string table_name;
        sqlite3 * db;
        sqlite3_stmt *query_stmt;
        char record_buffer[102400]{};
        uint64 current_record_id{};
        uint64 current_record_size{};
    public:
        record_iterator(const record_iterator&) = delete;
        record_iterator(sqlite3 * db, sqlite3_stmt* query_stmt,std::string table_name);
        ~record_iterator();
        [[nodiscard]] bool next() ;
        [[nodiscard]] uint64 get_record_size() const;
        uint64 get_record_data(void *buffer) const;
        int update_record(void * data,uint64 size) const;
        int delete_record();
    };
    class db_data_manager {
    private:
        std::string path;
        sqlite3* db;
        db_data_manager(std::string path,sqlite3* db);
    public:
        ~db_data_manager();
        static std::unique_ptr<db_data_manager> open(const char *path);
        int create_table(table * new_table);
        int insert_record(const char * table_name,const void* buffer,uint64 size);
        record_iterator get_record_iterator_write_transaction(const char * table_name);
    };
}




#endif //DB_DATA_MANAGER_H