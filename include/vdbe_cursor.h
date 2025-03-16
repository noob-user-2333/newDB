//
// Created by user on 25-3-5.
//

#ifndef VDBE_CURSOR_H
#define VDBE_CURSOR_H
#include "table.h"
#include "btree.h"
#include "dbManager.h"
#include "../third-part/sqlite/sqlite3.h"
namespace iedb
{
    class vdbe_cursor
    {
    private:
        btree* tree;
        table* _table;
        std::unique_ptr<btree::cursor> cursor;
        std::vector<uint64> delete_keys;
        bool writable = false;
        vdbe_cursor(btree * tree,table*_table);
    public:
        static std::unique_ptr<vdbe_cursor> open(const std::string& table_name);
        static int create_table(std::unique_ptr<table> & _table);
        int row_to_record(const std::vector<column_value> & row,std::vector<uint8>& record) const;
        [[nodiscard]] constexpr const table&get_table() const {return *_table;}
        void record_to_row(const std::vector<uint8> &record,std::vector<column_value> & row);
        int begin_read();
        int next();
        int get_record(uint64& out_rowid,std::vector<uint8>&out_record);
        int begin_write();
        int insert(uint64 key,const void*buffer,int size);
        void delete_record(uint64 key);
        int commit();


    };







}
#endif //VDBE_CURSOR_H
