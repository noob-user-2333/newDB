//
// Created by user on 25-2-26.
//
#include "btree_internal_page.h"
#include "test.h"

namespace iedb
{
    static uint64 random[1024 * 1024];
    static uint8 page_buffer[page_size];
    static constexpr char path[] = "/dev/shm/btree_internal_page";
    TEST(btree_internal_page, init)
    {
        // auto f = fopen(path,"r");
        // assert(fread(random,1, sizeof(random),f) == sizeof(random));
        // fclose(f);
        test::get_random(random,sizeof(random));
        test::save_data_to_file(random,sizeof(random),path);
        auto next_page = static_cast<int>(random[0]);
        auto prev_page = static_cast<int>(random[1]);
        auto p = btree_internal_page::init(page_buffer, prev_page, next_page);
        // ASSERT_EQ(p->checksum,os::calculate_checksum(p,sizeof(btree_page) - sizeof(uint64)));
        ASSERT_EQ(p->type, btree_page_type::internal);
        ASSERT_EQ(p->next_page, next_page);
        ASSERT_EQ(p->prev_page, prev_page);
        ASSERT_EQ(p->key_count, 0);
    }

    TEST(btree_internal_page, insert)
    {
        auto p = btree_internal_page::open(page_buffer);

    }

    TEST(btree_internal_page,remove)
    {

    }

    TEST(btree_internal_page, balance)
    {

    }
    TEST(btree_internal_page,merge)
    {

    }







}



