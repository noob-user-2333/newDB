//
// Created by user on 25-3-5.
//
#include "vdbe_cursor.h"

namespace iedb
{
    vdbe_cursor::vdbe_cursor(sqlite3* db, std::unique_ptr<table>& _table, sqlite3_stmt* stmt):_table(std::move(_table))
    {
        this->db = db;
        this->stmt = stmt;
    }
    sqlite3* vdbe_cursor::get_sqlite()
    {
        static sqlite3* db = nullptr;
        static char init_sql[] = "CREATE TABLE IF NOT EXISTS tables(name TEXT unique,content BLOB NOT NULL);";
        auto& manager = dbManager::get_instance();
        auto name = manager.get_root_dir() + "/vdbe.sqlite";
        if (db == nullptr)
        {
            assert(sqlite3_open(name.c_str(), &db) == SQLITE_OK);
            assert(sqlite3_exec(db,init_sql,nullptr,nullptr,nullptr) == SQLITE_OK);
        }

        return db;
    }
    std::unique_ptr<table> vdbe_cursor::get_table(const std::string& table_name)
    {
        static char buffer[1024];
        auto len = sprintf(buffer, "SELECT content FROM tables WHERE name = '%s';", table_name.c_str());
        auto db = get_sqlite();
        //首先查询是否存在对应名称表
        sqlite3_stmt *query_stmt;
        auto status = sqlite3_prepare_v2(db,buffer,len,&query_stmt,nullptr);
        assert(status == SQLITE_OK);
        if (sqlite3_step(query_stmt) != SQLITE_ROW)
            return nullptr;
        auto content = sqlite3_column_blob(query_stmt,0);
        auto size = sqlite3_column_bytes(query_stmt,0);
        auto _table = table::get(content,size);
        sqlite3_finalize(query_stmt);
        return std::move(_table);
    }

    std::unique_ptr<vdbe_cursor> vdbe_cursor::open(const std::string& table_name)
    {
        static char buffer[1024];
        auto _table = get_table(table_name);
        if (_table == nullptr)
        {
            fprintf(stderr,"不存在表:%s\n",table_name.c_str());
            return nullptr;
        }
        //查询表数据
        auto db =get_sqlite();
        sqlite3_stmt* query_stmt;

        return std::unique_ptr<vdbe_cursor>(new vdbe_cursor(db,_table,nullptr));
    }

    int vdbe_cursor::create_table(const table& _table)
    {
        static std::vector<uint8> buffer;
        static char sql[4096];
        auto& name = _table.get_name();
        if (get_table(name))
            return status_table_exists;
        auto size = table::translate_to_buffer(_table,buffer);
        auto len = sprintf(sql,"CREATE TABLE %s (id INTEGER UNIQUE,content BLOB);",name.c_str());
        assert(sqlite3_exec(get_sqlite(),sql,nullptr,nullptr,nullptr) == SQLITE_OK);
        sqlite3_stmt*stmt;
        len = sprintf(sql,"INSERT INTO tables VALUES('%s',?);",name.c_str());
        assert(sqlite3_prepare(get_sqlite(),sql,len,&stmt,nullptr) == SQLITE_OK);
        assert(sqlite3_bind_blob(stmt,1,buffer.data(),buffer.size(),nullptr) == SQLITE_OK);
        assert(sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return status_ok;
    }

    int vdbe_cursor::begin_read()
    {
        static char buffer[1024];
        assert(writable == false &&stmt == nullptr);
        auto len = sprintf(buffer, "SELECT id,content FROM %s ORDER BY id ASC;", _table->get_name().c_str());
        auto status = sqlite3_prepare_v2(db,buffer,len,&stmt,nullptr);
        assert(status == SQLITE_OK);
        writable = false;
        return status_ok;
    }
    int vdbe_cursor::begin_write()
    {
        static char buffer[1024];
        assert(stmt == nullptr);
        auto status = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        assert(status == SQLITE_OK);
        writable = true;
        return status_ok;
    }

    int vdbe_cursor::row_to_record(const std::vector<column_value>& row, std::vector<uint8>& record) const {
        return _table->store_row_to_record(row,record);
    }
    void vdbe_cursor::record_to_row(const std::vector<uint8>& record, std::vector<column_value>& row)
    {
        _table->load_row_from_record(record,row);
    }

    int vdbe_cursor::insert(uint64 key, const void* buffer, int size)
    {
        static char insert_sql[4096];
        // assert(writable);
        sprintf(insert_sql,"INSERT INTO %s VALUES(?,?);", _table->get_name().c_str());
        assert(sqlite3_prepare_v2(db,insert_sql, sizeof(insert_sql), &stmt, nullptr) == SQLITE_OK);
        assert(sqlite3_bind_int64(stmt,1,key) == SQLITE_OK);
        assert(sqlite3_bind_blob(stmt,2,buffer,size,nullptr) == SQLITE_OK);
        assert(sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        stmt = nullptr;
        return status_ok;
    }
    void vdbe_cursor::delete_record(uint64 key)
    {
        delete_keys.push_back(key);
    }

    int vdbe_cursor::commit()
    {
        if (writable)
        {
            if (!delete_keys.empty())
            {
                static char delete_sql[4096];
                sprintf(delete_sql,"DELETE FROM %s WHERE id=?;", _table->get_name().c_str());
                assert(sqlite3_prepare_v2(db, delete_sql, sizeof(delete_sql), &stmt, nullptr) == SQLITE_OK);
                for (auto key : delete_keys)
                {
                    assert(sqlite3_bind_int64(stmt,1,key) == SQLITE_OK);
                    assert(sqlite3_step(stmt) == SQLITE_DONE);
                }
                sqlite3_finalize(stmt);
                stmt = nullptr;
                delete_keys.clear();
            }
            assert(sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr) == SQLITE_OK);
        }else
        {
            sqlite3_finalize(stmt);
        }
        stmt = nullptr;
        writable = false;
        return status_ok;
    }

    int vdbe_cursor::next()
    {
        assert(stmt);
        if (sqlite3_step(stmt) == SQLITE_ROW)
            return status_ok;
        return status_out_of_range;
    }

    int vdbe_cursor::get_record(uint64&out_rowid,std::vector<uint8>& out_record)
    {
        out_rowid = sqlite3_column_int64(stmt,0);
        out_record.resize(sqlite3_column_bytes(stmt,1));
        memcpy(out_record.data(),sqlite3_column_blob(stmt,1),out_record.size());
        return status_ok;
    }





}