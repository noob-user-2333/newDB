//
// Created by user on 25-3-5.
//
#include "vdbe_cursor.h"

namespace iedb
{
    vdbe_cursor::vdbe_cursor(btree *tree, table *_table):tree(tree),_table(_table),cursor(nullptr) {

    }

    std::unique_ptr<vdbe_cursor> vdbe_cursor::open(const std::string& table_name)
    {
        static char buffer[1024];
        dbManager::dbTable* dbTable;
        auto status = dbManager::get_dbTable(table_name,dbTable);
        if (status != status_ok) {
            fprintf(stderr, "vdbe_cursor::get_table cannot found table %s\n", table_name.c_str());
            return nullptr;
        }
        return std::unique_ptr<vdbe_cursor>(new vdbe_cursor(dbTable->tree.get(),dbTable->_table.get()));
    }

    int vdbe_cursor::create_table(std::unique_ptr<table> &_table)
    {
        return dbManager::create_dbTable(_table);
    }

    int vdbe_cursor::begin_read()
    {
        assert(writable == false);
        return tree->get_cursor(0,cursor);
    }
    int vdbe_cursor::begin_write()
    {
        static char buffer[1024];
        assert(tree->enable_write() == status_ok);
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
        assert(tree->enable_write() == status_ok);
        assert(tree->insert(key,{(void*)buffer,(uint64)size}) == status_ok);
        assert(tree->commit() == status_ok);
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
                std::unique_ptr<btree::cursor> _cursor;
                memory_slice slice{};
                uint64 _key;
                for (auto key : delete_keys)
                {
                    auto status = tree->get_cursor(key,_cursor);
                    if (status != status_ok)
                        return status;
                    _cursor->get_item(_key,slice);
                    assert(key == _key);
                    status = _cursor->remove();
                    if (status!= status_ok)
                        return status;
                }
                cursor = nullptr;
                delete_keys.clear();
            }
            auto status = tree->commit();
            return status;
        }
        writable = false;
        return status_ok;
    }

    int vdbe_cursor::next()
    {
        return cursor->next();
    }

    int vdbe_cursor::get_record(uint64&out_rowid,std::vector<uint8>& out_record)
    {
        memory_slice slice{};
        cursor->get_item(out_rowid,slice);
        out_record.resize(slice.size);
        memcpy(out_record.data(),slice.buffer,slice.size);
        return status_ok;
    }





}