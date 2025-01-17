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
        int64 current_record_id{};
        int64 current_record_size{};
        const void * current_record_data{};

    public:
        record_iterator(const record_iterator&) = delete;
        record_iterator(sqlite3 * db, sqlite3_stmt* query_stmt,std::string table_name);
        ~record_iterator();
        [[nodiscard]] bool next();
        [[nodiscard]] uint64 get_record_size() const;
        [[nodiscard]] const void* get_record_data() const;
        int update_record(const void * data,uint64 size) const;
        int delete_record() const;
    };
    class db_data_manager {
    private:
        std::string path;
        sqlite3* db;
        std::unordered_map<std::string,std::unique_ptr<table>> table_map;
        db_data_manager(std::string path,sqlite3* db,std::unordered_map<std::string,std::unique_ptr<table>>&map);
    public:
        ~db_data_manager();
        static std::unique_ptr<db_data_manager> open(const char *path);
        int create_table(std::unique_ptr<table> new_table);
        const table* get_table(const std::string & table_name);
        int insert_record(const std::string& table_name,const void* buffer,uint64 size);
        std::unique_ptr<record_iterator> get_record_iterator_write_transaction(const char * table_name);
    };
}




#endif //DB_DATA_MANAGER_H
