//
// Created by user on 25-1-5.
//
#include "db_data_manager.h"

#include <iostream>

#include "libbase64.h"
namespace iedb {
    record_iterator::record_iterator(sqlite3 *db, sqlite3_stmt *query_stmt, std::string table_name) :
        db(db), query_stmt(query_stmt), table_name(std::move(table_name)) {}
    record_iterator::~record_iterator() { sqlite3_finalize(query_stmt); }
    uint64 record_iterator::get_record_size() const { return current_record_size * 3 / 4; }
    uint64 record_iterator::get_record_data(void *buffer) const {
        uint64 record_size;
        base64_decode(record_buffer, current_record_size, static_cast<char *>(buffer), &record_size,
                      BASE64_FORCE_AVX512);
        return record_size;
    }

    bool record_iterator::next() {
        if (sqlite3_step(query_stmt) == SQLITE_ROW) {
            current_record_id = sqlite3_column_int64(query_stmt, 0);
            current_record_size = sqlite3_column_bytes(query_stmt, 1);
            if (auto data = sqlite3_column_text(query_stmt, 1)) {
                memcpy(record_buffer, data, current_record_size);
            }
            return true;
        }
        return false;
    }
    int record_iterator::update_record(void *data, uint64 size) const {
        static char sql_buff[1024 * 1024 * 16];
        int64 offset = sprintf(sql_buff, "UPDATE %s SET record = '");
        uint64 len;
        base64_encode(static_cast<const char *>(data), size, sql_buff + offset, &len, BASE64_FORCE_AVX512);
        sprintf(sql_buff + offset + len, "' WHERE id = %ld;", current_record_id);
        char *errmsg;
        if (sqlite3_exec(db, sql_buff, nullptr, nullptr, &errmsg) != SQLITE_OK) {
            fprintf(stderr, "%s", errmsg);
            return status_error;
        }
        return status_ok;
    }
    int record_iterator::delete_record() {
        static char sql_buff[1024 * 1024 * 16];
        sprintf(sql_buff, "DELETE FROM %s WHERE id = %ld;", table_name.c_str(), current_record_id);
        char *errmsg;
        if (sqlite3_exec(db, sql_buff, nullptr, nullptr, &errmsg) != SQLITE_OK) {
            fprintf(stderr, "%s", errmsg);
            return status_error;
        }
        return status_ok;
    }

    db_data_manager::db_data_manager(std::string path, sqlite3 *db) : path(std::move(path)), db(db) {}
    db_data_manager::~db_data_manager() { sqlite3_close_v2(db); }
    std::unique_ptr<db_data_manager> db_data_manager::open(const char *path) {
        sqlite3 *db;
        if (int rc = sqlite3_open(path, &db)) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            return nullptr;
        }
        // 创建用于保存数据的master表和data表
        static constexpr char create_master[] = "CREATE TABLE IF NOT EXISTS master ("
                                                "name TEXT NOT NULL UNIQUE,"
                                                "format TEXT NOT NULL);";

        sqlite3_exec(db, create_master, nullptr, nullptr, nullptr);
        return std::unique_ptr<db_data_manager>(new db_data_manager(path, db));
    }
    int db_data_manager::create_table(table *new_table) {
        static constexpr char create_record_table_sql[] = "CREATE TABLE %d("
                                                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                          "record TEXT NOT NULL);";
        static char buffer[10240];
        // 先向master中插入表定义
        // 然后创建保存记录的表
        sprintf(buffer, create_record_table_sql, new_table->get_name().c_str());
        sqlite3_exec(db, buffer, nullptr, nullptr, nullptr);
        return status_ok;
    }


    int db_data_manager::insert_record(const char *table_name, const void *buffer, uint64 size) {
        static char sql_buffer[1024 * 1024];
        uint64 out_size;
        auto offset = sprintf(sql_buffer, "INSERT INTO %s (record) VALUES ('", table_name);
        base64_encode(static_cast<const char *>(buffer), size, sql_buffer + offset, &out_size, BASE64_FORCE_AVX512);
        sprintf(sql_buffer + offset + out_size, "');");
        sqlite3_exec(db, sql_buffer, nullptr, nullptr, nullptr);
        return status_ok;
    }


    record_iterator db_data_manager::get_record_iterator_write_transaction(const char *table_name) {
        static char buffer[10240];
        sqlite3_stmt *query_stmt;
        const auto len = sprintf(buffer, "SELECT record FROM %s ORDER BY id;", table_name);
        if (sqlite3_prepare_v2(db, buffer, len, &query_stmt, nullptr) != SQLITE_OK) {
            std::cerr << "查询预编译失败: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(query_stmt);
            sqlite3_close(db);
            throw std::runtime_error("无法正常执行查询操作");
            }
            return {db,query_stmt,table_name};
    }

}