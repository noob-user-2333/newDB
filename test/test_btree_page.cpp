//
// Created by user on 25-2-18.
//

#include "../include/btree.h"
#include "test.h"

#include "btree_leaf_page.h"

namespace iedb
{
    static uint64 random[1024 * 1024];
    static uint8 page_buffer[page_size];
    TEST(btree_page, init)
    {
        auto f = fopen("/dev/shm/data","r");
        assert(fread(random,1, sizeof(random),f) == sizeof(random));
        fclose(f);
        // test::get_random(random,sizeof(random));
        // test::save_data_to_file(random,sizeof(random),"/dev/shm/data");
        auto next_page = static_cast<int>(random[0]);
        auto prev_page = static_cast<int>(random[1]);
        auto p = btree_leaf_page::init(page_buffer, prev_page, next_page);
        // ASSERT_EQ(p->checksum,os::calculate_checksum(p,sizeof(btree_page) - sizeof(uint64)));
        ASSERT_EQ(p->type, btree_page_type::leaf);
        ASSERT_EQ(p->next_page, next_page);
        ASSERT_EQ(p->prev_page, prev_page);
        ASSERT_EQ(p->free_fragment_count, 0);
        ASSERT_EQ(p->payload_count, 0);
        ASSERT_EQ(p->free_zone_offset, sizeof(btree_leaf_page));
        ASSERT_EQ(p->data_zone_offset, page_size);
        ASSERT_EQ(p->free_fragment_offset, 0);
        ASSERT_EQ(p->payload_size_count, 0);
    }

    TEST(btree_page, insert)
    {
        auto p = btree_leaf_page::open(page_buffer);
        auto cursor = p->get_cursor();
        auto status = status_ok;
        auto payload_size_count = 0;
        memory_slice slice{random, 0};
        int i, j, k;
        for (i = 0; status == status_ok; i++)
        {
            auto insert_data_size = static_cast<int>(random[i] % (page_size / 128));
            auto key = random[i];
            if (insert_data_size == 0)
                insert_data_size = 1;
            slice.buffer = random;
            slice.size = insert_data_size;
            payload_size_count += (static_cast<int>(slice.size) + 7) & (~7);
            status = cursor.insert_payload(key, slice);
            if (status == status_ok)
            {
                ASSERT_EQ(p->payload_count, i + 1);
                ASSERT_EQ(p->payload_size_count, payload_size_count);
                //每隔一定次数检查一次key顺序
                if ((i % 128) == 0)
                    for (j = 1; j < p->payload_count; j++)
                    {
                        EXPECT_LE(p->payloads[j - 1].key, p->payloads[j].key);
                    }
            }
        }
        //首先检查payloads中key的顺序
        for (j = 1; j < p->payload_count; j++)
        {
            ASSERT_LE(p->payloads[j - 1].key, p->payloads[j].key);
        }
        //检验payloads中的数据是否正常
        ASSERT_EQ(cursor.first(), status_ok);
        ASSERT_EQ(p->payload_count , i - 1);
        for (k = 0; k < p->payload_count; k++)
        {
            uint64 key;
            cursor.get_payload(key, slice);
            ASSERT_EQ(memcmp(slice.buffer,random,slice.size), 0);
            if (k < p->payload_count - 1)
                ASSERT_LT(p->payloads[k].key,p->payloads[k + 1].key);
                // ASSERT_LE(p->payloads[k+1].offset, p->payloads[k].offset);
            cursor.next();
        }
        ASSERT_EQ(cursor.next(), status_out_of_range);
    }

    TEST(btree_page, delete)
    {
        auto p = btree_leaf_page::open(page_buffer);
        auto cursor = p->get_cursor();
        auto last_fragment_offset = 0;
        auto last_payload_size_count = 0;
        auto last_fragment_size_count = 0;
        auto ori_payload_count = p->payload_count;
        auto offset = 0;
        uint64 key;
        memory_slice slice{random, 0};
        ASSERT_EQ(p->free_fragment_offset, 0);
        //首先确定删除数据的数量
        auto delete_count = random[0] % p->payload_count;
        for (auto times = 0; times < delete_count; times++)
        {
            ASSERT_EQ(cursor.first(), status_ok);
            last_payload_size_count = p->payload_size_count;
            last_fragment_offset = p->free_fragment_offset;
            last_fragment_size_count = p->free_fragment_count;
            auto ori_data_zone = p->data_zone_offset;
            //确定删除数据索引值
            auto index = static_cast<int>(random[times] % p->payload_count) - 1;
            if (index < 0)
                index = 0;
            //移动到需要删除的数据的位置
            for (auto i = 0; i < index; i++)
                ASSERT_EQ(cursor.next(), status_ok);
            //获取待删除数据的偏移量和大小
            cursor.get_payload(key, slice);
            offset = static_cast<int>(reinterpret_cast<uint64>(slice.buffer) - reinterpret_cast<uint64>(page_buffer));
            auto actual_slice_size = (slice.size + 7) & (~7);
            //删除数据
            ASSERT_EQ(cursor.delete_payload(), status_ok);
            ASSERT_EQ(ori_payload_count - 1 - times, p->payload_count);
            //检查payload_size_count和free_fragment_size_count是否正常
            ASSERT_EQ(last_payload_size_count - actual_slice_size, p->payload_size_count);
        }
        //检验key值是否正常排列
        for (auto i = 0; i < p->payload_count - 1; i++)
            ASSERT_LE(p->payloads[i].key, p->payloads[i + 1].key);
        //检验数据是否正常
        ASSERT_EQ(cursor.first(), status_ok);
        for (auto i = 0; i < p->payload_count; i++)
        {
            cursor.get_payload(key, slice);
            ASSERT_EQ(memcmp(slice.buffer, random, slice.size), 0);
            cursor.next();
        }
    }

    TEST(btree_page, update)
    {
        auto p = btree_leaf_page::open(page_buffer);
        auto cursor = p->get_cursor();
        auto last_fragment_offset = 0;
        auto last_payload_size_count = 0;
        auto last_fragment_size_count = 0;
        auto ori_payload_count = p->payload_count;
        auto offset = 0;
        uint64 key;
        memory_slice slice{random, 0};
        memory_slice new_slice{};
        //全部修改一次
        ASSERT_EQ(cursor.first(), status_ok);
        auto update_count = p->payload_count;
        for (auto times = 1; times < update_count; times++)
        {
            cursor.get_payload(key, slice);
            new_slice.set(random + slice.size, slice.size);
            cursor.update_payload(new_slice);
            //检验是否正常更新数据
            cursor.get_payload(key, slice);
            ASSERT_EQ(memcmp(slice.buffer, random + slice.size, slice.size), 0);
            ASSERT_EQ(cursor.next(), status_ok);
        }
        //随机挑选数据进行修改
        auto mod_num = (p->payload_count - 2);
        if (mod_num < 2)
            mod_num = 2;
        update_count = static_cast<int>(random[1]) % mod_num;
        for (auto times = 0; times < update_count; times++)
        {
            auto index = static_cast<int>(random[times] % p->payload_count) - 2;
            if (index < 0)
                index = 0;
            //前往指定数据处
            ASSERT_EQ(cursor.first(), status_ok);
            for (auto i = 0; i < index; i++)
                ASSERT_EQ(cursor.next(), status_ok);
            //对数据进行修改
            auto size = (index + 1) * 8;
            new_slice.set(random, size);
            auto status = cursor.update_payload(new_slice);
            if (status != status_ok)
                break;
            //检验是否正常更新数据
            cursor.get_payload(key, slice);
            ASSERT_EQ(memcmp(slice.buffer, random, slice.size), 0);
            cursor.next();
        }
        //检验key值是否正常排列
        for (auto i = 0; i < p->payload_count - 1; i++)
            ASSERT_LE(p->payloads[i].key, p->payloads[i + 1].key);
        //检验数据是否正常
        ASSERT_EQ(cursor.first(), status_ok);
        for (auto i = 0; i < p->payload_count; i++)
        {
            cursor.get_payload(key, slice);
            const auto cmp1 = memcmp(slice.buffer, random, slice.size);
            const auto cmp2 = memcmp(slice.buffer, random + slice.size, slice.size);
            // ASSERT_FALSE(cmp2 && cmp1);
            ASSERT_TRUE(cmp1 == 0 || cmp2 == 0);
            cursor.next();
        }
    }

    // TEST(btree_page_test,merge)
    // {
    //
    // }
    TEST(btree_page, balance)
    {
        static uint8 balance_buffer[2][page_size];
        auto front_page = btree_leaf_page::init(balance_buffer[0],0,0);
        auto back_page = btree_leaf_page::init(balance_buffer[1],0,0);
        auto front_cursor = front_page->get_cursor();
        auto back_cursor = back_page->get_cursor();
        auto status = status_ok;
        auto front_insert_size = static_cast<int>(random[0] % (page_size / 4 * 3));
        auto back_insert_size = static_cast<int>(random[1] % (page_size / 4 * 3));
        if (front_insert_size < page_size / 8)
            front_insert_size = page_size / 8;
        if (back_insert_size < page_size / 8)
            back_insert_size = page_size / 8;

        memory_slice slice{random, 0};
        std::map<uint64,int> map;
        uint64 key;
        int insert_data_size;
        auto insert_count = 0;
        //随机生成一定数量数据
        for (auto i = 0; i < 1024 * 4; i++)
        {
            do
            {
                insert_data_size = static_cast<int>(random[i] % (page_size / 128));
                key = random[i];
                if (insert_data_size == 0)
                    insert_data_size = 1;
                i++;
                auto it = map.find(key);
                if (it == map.end())
                {
                    map[key] = insert_data_size;
                    break;
                }
            }while (true);
        }
        //开始插入
        auto it = map.begin();
        while (front_page->payload_size_count < front_insert_size)
        {
            key = it->first;
            slice.size = it->second;
            ASSERT_EQ(front_cursor.insert_payload(key,slice),status_ok);
            insert_count++;
            ++it;
        }
        while (back_page->payload_size_count < back_insert_size)
        {
            key = it->first;
            slice.size = it->second;
            ASSERT_EQ(back_cursor.insert_payload(key,slice),status_ok);
            ++it;
            insert_count++;
        }
        //平衡
        btree_leaf_page::balance(front_page, back_page,key);
        ASSERT_EQ(back_page->payloads[0].key,key);
        //检验key是否按正常顺序排列
        it = map.begin();
        front_cursor = front_page->get_cursor();
        back_cursor = back_page->get_cursor();
        auto count = 0;
        while (front_cursor.next() == status_ok)
        {
            front_cursor.get_payload(key,slice);
            ASSERT_EQ(it->first, key);
            ASSERT_EQ(it->second,slice.size);
            ++it;
            count++;
        }
        while (back_cursor.next() == status_ok)
        {
            back_cursor.get_payload(key,slice);
            ASSERT_EQ(it->first, key);
            ASSERT_EQ(it->second,slice.size);
            ++it;
            count++;
        }
        ASSERT_EQ(count,insert_count);
    }
    TEST(btree_page,merge)
    {
        static uint8 balance_buffer[2][page_size];
        auto front_page = btree_leaf_page::init(balance_buffer[0],0,0);
        auto back_page = btree_leaf_page::init(balance_buffer[1], 0,0);
        auto front_cursor = front_page->get_cursor();
        auto back_cursor = back_page->get_cursor();
        auto status = status_ok;
        static constexpr auto target_insert_size = page_size / 2 / 8 * 7;
        memory_slice slice{random, 0};
        std::map<uint64,int> map;
        uint64 key;
        int insert_data_size;
        auto insert_count = 0;
        //随机生成一定数量数据
        for (auto i = 0; i < 1024 * 4; i++)
        {
            do
            {
                insert_data_size = static_cast<int>(random[i] % (page_size / 128));
                key = random[i];
                if (insert_data_size == 0)
                    insert_data_size = 1;
                i++;
                auto it = map.find(key);
                if (it == map.end())
                {
                    map[key] = insert_data_size;
                    break;
                }
            }while (true);
        }
        //开始插入
        auto it = map.begin();
        while (front_page->payload_size_count < target_insert_size)
        {
            key = it->first;
            slice.size = it->second;
            ASSERT_EQ(front_cursor.insert_payload(key,slice),status_ok);
            insert_count++;
            ++it;
        }
        while (back_page->payload_size_count < target_insert_size)
        {
            key = it->first;
            slice.size = it->second;
            ASSERT_EQ(back_cursor.insert_payload(key,slice),status_ok);
            ++it;
            insert_count++;
        }
        //合并
        btree_leaf_page::merge(front_page, back_page);
        //检验key是否按正常顺序排列
        it = map.begin();
        front_cursor = front_page->get_cursor();
        auto count = 0;
        while (front_cursor.next() == status_ok)
        {
            front_cursor.get_payload(key,slice);
            ASSERT_EQ(it->first, key);
            ASSERT_EQ(it->second,slice.size);
            ++it;
            count++;
        }
        ASSERT_EQ(count,insert_count);
        // back_cursor = back_page->get_cursor();
        // while (back_cursor.next() == status_ok)
        // {
            // back_cursor.get_payload(key, slice);
            // ASSERT_EQ(it->first, key);
            // ASSERT_EQ(it->second, slice.size);
            // ++it;
        // }
    }
}
