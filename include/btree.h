//
// Created by user on 25-2-14.
//

#ifndef BTREE_H
#define BTREE_H
#include "pager.h"
#include "btree_page.h"

namespace iedb
{
    class btree
    {
    public:

        class cursor
        {
        private:
            btree const & owner;
            btree_page * page;
            uint64 key;
            memory_slice data;
        public:
            cursor(cursor&) = delete;
            // int insert
        };
    private:
        std::unique_ptr<pager> _pager;



    public:
        btree(const btree&) = delete;
        btree(btree&&) = delete;
        ~btree() = default;
        btree(std::unique_ptr<pager>& _pager);
        cursor get_read_cursor(uint64 key);
        cursor get_write_cursor(uint64 key);
    };




}

#endif //BTREE_H
