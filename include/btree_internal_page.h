//
// Created by user on 25-2-26.
//

#ifndef BTREE_INTERNAL_PAGE_H
#define BTREE_INTERNAL_PAGE_H
#include "btree_leaf_page.h"
namespace iedb
{
    struct btree_internal_page
    {
        static constexpr int key_capacity = (page_size - sizeof(btree_page_type) - sizeof(int)*4)
                                / (sizeof(int) + sizeof(uint64));
        static constexpr int page_capacity = key_capacity+1;

        btree_page_type type;
        int key_count;
        int next_page;
        int prev_page;
        std::array<uint64,key_capacity> keys;
        std::array<int,page_capacity> pages_no;

        static btree_internal_page* init(void* data,int prev_page,int next_page);
        static btree_internal_page* open(void* data);
        static int merge(btree_internal_page * front_page, btree_internal_page * back_page,uint64 middle_key);
        static void split(btree_internal_page * front_page,void* back_page_buff,int front_page_no,int back_page_no,uint64&out_key);
        static void balance(btree_internal_page * front_page,btree_internal_page * back_page,uint64 middle_key,uint64&out_new_middle_key);
        int insert(uint64 key,int front_page_no,int back_page_no);
        void search(uint64 key,int & out_page_no) const;
        int remove(int page_no);
    };
}


#endif //BTREE_INTERNAL_PAGE_H
