//
// Created by user on 25-2-14.
//

#ifndef BTREE_H
#define BTREE_H
#include "pager.h"

namespace iedb
{
    class btree
    {
    public:

        class cursor{};
    private:
        struct btree_page;
        std::unique_ptr<pager> pager;



    public:
        btree(const btree&) = delete;
        btree(btree&&) = delete;



    };




}

#endif //BTREE_H
