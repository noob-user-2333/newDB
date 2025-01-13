//
// Created by user on 25-1-5.
//
#include "db_data_manager.h"

#include <iostream>

namespace iedb {
    record_iterator::record_iterator(sqlite3 *db, sqlite3_stmt *query_stmt, std::string table_name) :
        db(db), query_stmt(query_stmt), table_name(std::move(table_name)) {}
    record_iterator::~record_iterator() { sqlite3_finalize(query_stmt); }

    uint64 record_iterator::get_record_size() const
    {
        return current_record_size;
    }
    const void* record_iterator::get_record_data() const {
        return current_record_data;
    }

    bool record_iterator::next()
    {
        if (sqlite3_step(query_stmt) == SQLITE_ROW) {
            current_record_id = sqlite3_column_int64(query_stmt, 0);
            current_record_size = sqlite3_column_bytes(query_stmt,1);
            current_record_data = sqlite3_column_blob(query_stmt,1);
            return true;
        }
        return false;
    }
    int record_iterator::update_record(const void *data, uint64 size) const {
        static char sql_buff[1024];
        sqlite3_stmt * stmt;
        auto offset = sprintf(sql_buff, "UPDATE %s SET record = (?) WHERE id = %ld;",table_name.c_str(),current_record_id);
        auto rc = sqlite3_prepare_v2(db,sql_buff, offset, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return 1;
        }
        // 绑定 BLOB 数据到第一个占位符
        sqlite3_bind_blob(stmt, 1, data, size, SQLITE_STATIC);
        // 执行更新操作
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return status_error;
        }
        sqlite3_finalize(stmt);
        return status_ok;
    }
    int record_iterator::delete_record() const
    {
        static char sql_buff[1024];
        const auto id =current_record_id;
        sprintf(sql_buff, "DELETE FROM %s WHERE id = %ld;", table_name.c_str(),id);
        char *errmsg;
        if (sqlite3_exec(db, sql_buff, nullptr, nullptr, &errmsg) != SQLITE_OK) {
            fprintf(stderr, "%s", errmsg);
            return status_error;
        }
        return status_ok;
    }

    db_data_manager::db_data_manager(std::string path, sqlite3 *db,std::unordered_map<std::string,std::unique_ptr<table>>&map) : path(std::move(path)), db(db),table_map(std::move(map)) {}
    db_data_manager::~db_data_manager() { sqlite3_close_v2(db); }
    std::unique_ptr<db_data_manager> db_data_manager::open(const char *path) {
        sqlite3 *db;
        if (int rc = sqlite3_open(path, &db)) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            return nullptr;
        }
        // 创建用于保存数据的master表
        static constexpr char create_master[] = "CREATE TABLE IF NOT EXISTS master ("
                                                "name char(32) NOT NULL UNIQUE,"
                                                "format blob NOT NULL);";
        sqlite3_exec(db, create_master, nullptr, nullptr, nullptr);
        //查询所有master表以获取当前表定义
        static constexpr char query_master_tables[] = "SELECT name,format FROM master;";
        sqlite3_stmt *query_stmt;
        auto rc = sqlite3_prepare_v2(db, query_master_tables, -1, &query_stmt, nullptr);
        if (rc!= SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return nullptr;
        }
        std::unordered_map<std::string,std::unique_ptr<table>> map;
        while (sqlite3_step(query_stmt) == SQLITE_ROW)
        {
            auto name = std::string((const char*)sqlite3_column_text(query_stmt,0));
            auto format = sqlite3_column_blob(query_stmt,1);
            auto size = sqlite3_column_bytes(query_stmt,1);
            auto _table = table::get(format,size);
            map[name] = std::move(_table);
        }
        sqlite3_finalize(query_stmt);
        return std::unique_ptr<db_data_manager>(new db_data_manager(path, db,map));
    }
    int db_data_manager::create_table(table&new_table) {
        static constexpr char create_record_table_sql[] = "CREATE TABLE %s("
                                                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                          "record blob NOT NULL);";
        static constexpr char insert_table_to_master[] = "INSERT INTO master (name,format) VALUES (?,?);";
        static char buffer[10240];
        //将内容序列化写入到buffer中
        auto size = table::translate_to_buffer(new_table,buffer);
        // 先向master中插入表定义
        sqlite3_stmt * stmt;
        sqlite3_prepare_v2(db, insert_table_to_master, sizeof(insert_table_to_master), &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, new_table.get_name().c_str(), new_table.get_name().size(), SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmt, 2, buffer, size, SQLITE_STATIC);
        auto rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
            return status_error;
        sqlite3_finalize(stmt);
        // 然后创建保存记录的表
        sprintf(buffer, create_record_table_sql, new_table.get_name().c_str());
        sqlite3_exec(db, buffer, nullptr, nullptr, nullptr);
        return status_ok;
    }


    int db_data_manager::insert_record(const std::string &table_name, const void *buffer, uint64 size) {
        static char sql_buffer[1024];
        sqlite3_stmt *insert_stmt;
        auto offset = sprintf(sql_buffer, "INSERT INTO %s (record) VALUES (?);", table_name.c_str());
        auto rc = sqlite3_prepare_v3(db,sql_buffer,offset,SQLITE_PREPARE_NORMALIZE,&insert_stmt,nullptr);
        if (rc!= SQLITE_OK)
        {
            std::cerr << "插入预编译失败: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(insert_stmt);
            sqlite3_close(db);
            throw std::runtime_error("无法正常执行插入操作");
        }
        sqlite3_bind_blob(insert_stmt, 1, buffer, size, SQLITE_STATIC);
        rc = sqlite3_step(insert_stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(insert_stmt);
            sqlite3_close(db);
            return status_error;
        }
        sqlite3_exec(db, sql_buffer, nullptr, nullptr, nullptr);
        return status_ok;
    }
    const table* db_data_manager::get_table(const std::string& table_name)
    {
        auto it = table_map.find(table_name);
        if (it!= table_map.end()) {
            return it->second.get();
        }
        return nullptr;
    }


    record_iterator db_data_manager::get_record_iterator_write_transaction(const char *table_name) {
        static char buffer[10240];
        sqlite3_stmt *query_stmt;
        const auto len = sprintf(buffer, "SELECT id,record FROM %s;", table_name);
        if (sqlite3_prepare_v2(db, buffer, len, &query_stmt, nullptr) != SQLITE_OK) {
            std::cerr << "查询预编译失败: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(query_stmt);
            sqlite3_close(db);
            throw std::runtime_error("无法正常执行查询操作");
            }
            return {db,query_stmt,table_name};
    }

}