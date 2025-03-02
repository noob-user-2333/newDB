//
// Created by user on 25-2-23.
//


#include <filesystem>
#include "btree.h"
#include "os.h"
#include "test.h"
#include <sys/time.h>
#include "../third-part/sqlite/sqlite3.h"
#include "../third-part/roaring_bitmap/roaring.h"
namespace iedb
{
    static std::string path = "/dev/shm/test/btree_test.iedb";
    static std::string sqlite_path = "/dev/shm/test/test.db";
    static constexpr char create_sql[] = "CREATE TABLE IF NOT EXISTS btree(id int UNIQUE ,data blob not null);";
    static constexpr char clear_sql[] = "DELETE FROM btree WHERE 1=1;";
    static constexpr char insert_sql[] = "INSERT INTO btree VALUES(?,?);";
    static constexpr char select_sql[] = "SELECT * FROM btree ORDER BY id ASC;";
    static constexpr uint64 key_mask = ~0x8000000000000000UL;
    static std::unique_ptr<btree> tree;
    static sqlite3* db;
    static sqlite3_stmt* stmt;
    static bool init_success = false;
    static constexpr char random_path[] = "/dev/shm/btree-random";
    static constexpr int max_page_count = 1024 * 64;
    static uint8 buffer[max_page_count * sizeof(uint64) * 2];
    static std::vector<uint64> key_vector;
    static int insert_count;
    TEST(btree, init)
    {
        // test::get_random(buffer, sizeof(buffer));
        // test::save_data_to_file(buffer,sizeof(buffer),random_path);
        test::read_file(random_path,0,sizeof(buffer),buffer);
        // 检查是否存在对应文件夹，如不存在则创建
        std::filesystem::path _path(path);
        std::filesystem::path directory = _path.parent_path();
        if (os::access(directory.c_str(),os::access_mode_file_exists) != status_ok)
            os::mkdir(directory.c_str());
        //删除对应文件，确保不受过去测试影响
        os::unlink(path.c_str());
        os::unlink((path + "-journal").c_str());
        //开启pager
        auto _pager = pager::open(path);
        ASSERT_TRUE(_pager);
        tree = btree::open(_pager);
        ASSERT_TRUE(tree);
        //检查开头16字节是否为对应字符串
        char head[16];
        test::read_file(path.c_str(), 0, sizeof(btree::btree_header::magic_string), head);
        ASSERT_EQ(memcmp(head, btree::btree_header::magic_string,sizeof(btree::btree_header::magic_string)), 0);
        //使用sqlite备份数据用于后续插入与删除数据后的查询比校
        ASSERT_EQ(sqlite3_open(sqlite_path.c_str(),&db), 0);
        //建表并清空数据
        ASSERT_EQ(sqlite3_exec(db, create_sql, nullptr, nullptr, nullptr), SQLITE_OK);
        ASSERT_EQ(sqlite3_exec(db, clear_sql, nullptr, nullptr, nullptr), SQLITE_OK);
        init_success = true;
    }

    TEST(btree, insert)
    {
        ASSERT_TRUE(init_success);
        auto random = buffer;
        auto uint64_random = reinterpret_cast<const uint64*>(random);
        insert_count =static_cast<int>(uint64_random[0] % max_page_count);
        auto index = 0;
        roaring::Roaring map;
        //准备sqlite3参数化插入
        sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        ASSERT_EQ(sqlite3_prepare_v2(db,insert_sql, sizeof(insert_sql), &stmt, nullptr), SQLITE_OK);
        // insert_count = 38;
        ASSERT_EQ(tree->enable_write(),status_ok);
        for (auto i = 0; i < insert_count;i++,index++)
        {
            //确定将要插入的key
            uint64 key = 0;
            int64 actual_key;
            do
            {
            key = uint64_random[index];
            actual_key = static_cast<int64>(key & key_mask);
            index++;
            }while(map.contains(actual_key));
            map.add(actual_key);
            assert(actual_key >= 0);
            assert(map.contains(actual_key));
            // struct timeval tv;
            // gettimeofday(&tv, NULL);  // 获取当前时间

            // long timestamp_us = tv.tv_sec * 1000000L + tv.tv_usec; // 转换为微秒级时间戳
            // actual_key = timestamp_us;
            assert(actual_key > 0);
            key_vector.push_back(actual_key);
            //确定插入数据长度
            auto size = static_cast<int>(actual_key % (page_size / 4));
            if (size == 0)
                size = 8;
            //插入数据
            ASSERT_EQ(sqlite3_bind_int64(stmt,1,actual_key),SQLITE_OK);
            ASSERT_EQ(sqlite3_bind_blob(stmt,2,buffer, size, nullptr),SQLITE_OK);
            ASSERT_EQ(sqlite3_step(stmt),SQLITE_DONE);
            sqlite3_reset(stmt);   // 复位语句，准备下一次插入
            sqlite3_clear_bindings(stmt); // 清除绑定值

            memory_slice slice{};
            slice.set((void*)(uint64_random),size);

        // ASSERT_EQ(tree->enable_write(),status_ok);
            ASSERT_EQ(tree->insert(actual_key,slice),status_ok);
        // ASSERT_EQ(tree->commit(),status_ok);
        }
        ASSERT_EQ(tree->commit(),status_ok);
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
        ASSERT_EQ(sqlite3_finalize(stmt), SQLITE_OK);

        //查询数据
        std::unique_ptr<btree::cursor> cursor;
        //准备数据
        ASSERT_EQ( tree->get_first_cursor(cursor),status_ok);
        ASSERT_EQ(sqlite3_prepare_v2(db,select_sql, sizeof(select_sql), &stmt, nullptr), SQLITE_OK);
        index = 0;
        // 逐条获取数据
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            index++;
            // ASSERT_LE(index,insert_count);
            //获取sqlite数据
            auto key = sqlite3_column_int64(stmt, 0);
            auto data = (sqlite3_column_blob(stmt, 1));
            auto data_size = sqlite3_column_bytes(stmt,1);
            //获取btree数据
            uint64 tree_key;
            memory_slice tree_data{};

            cursor->get_item(tree_key,tree_data);
            cursor->next();
            //比较是否一致
            if (key != tree_key)
                fprintf(stderr, "key not equal tree_key when index == %d\n",index);
            ASSERT_EQ(key, tree_key);
            ASSERT_EQ(data_size, tree_data.size);
            ASSERT_EQ(0, memcmp(data, tree_data.buffer, data_size));
        }
        // 释放资源
        sqlite3_finalize(stmt);
    }
    TEST(btree, delete)
    {
        ASSERT_TRUE(init_success);
        char sql[1024];
        auto random = buffer;
        auto uint64_random = reinterpret_cast<const uint64*>(random);
        auto delete_count = insert_count / 2;
        auto index = 0;
        auto start = key_vector.data() + delete_count;
        roaring::Roaring map;
        ASSERT_EQ(tree->enable_write(),status_ok);
        std::unique_ptr<btree::cursor> cursor;
        for (auto i = 0; i < delete_count; i++, index++)
        {
            auto key = start[i];
            auto tree_key = 0UL;
            memory_slice tree_data{};
            if (i >= 14)
            {
                ASSERT_EQ(tree->get_cursor(key,cursor),status_ok);
                //打印page-6中的key
                // dbPage_ref page_ref;
                // tree->_pager->get_page(6,page_ref);
                // auto page = tree->open_leaf_page(page_ref);
                // auto count = page->payload_count;
                // for (int j = 0; j < count; j++)
                //     printf("%ld ",page->payloads[j].key);
                // printf("\n");
            }
            else
                ASSERT_EQ(tree->get_cursor(key,cursor),status_ok);
            cursor->get_item(tree_key,tree_data);
            ASSERT_EQ(tree_key,key);
            ASSERT_EQ(cursor->remove(),status_ok);
            sprintf(sql,"DELETE  FROM btree WHERE id = %ld;",key);
            ASSERT_EQ(sqlite3_exec(db, sql, nullptr, nullptr, nullptr),SQLITE_OK);
        }
        ASSERT_EQ(tree->commit(),status_ok);
        //准备数据
        ASSERT_EQ( tree->get_first_cursor(cursor),status_ok);
        ASSERT_EQ(sqlite3_prepare_v2(db,select_sql, sizeof(select_sql), &stmt, nullptr), SQLITE_OK);
        index = 0;
        // 逐条获取数据
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            index++;
            // ASSERT_LE(index,insert_count);
            //获取sqlite数据
            auto key = sqlite3_column_int64(stmt, 0);
            auto data = (sqlite3_column_blob(stmt, 1));
            auto data_size = sqlite3_column_bytes(stmt,1);
            //获取btree数据
            uint64 tree_key;
            memory_slice tree_data{};

            cursor->get_item(tree_key,tree_data);
            cursor->next();
            //比较是否一致
            if (key != tree_key)
                fprintf(stderr, "key not equal tree_key when index == %d\n",index);
            ASSERT_EQ(key, tree_key);
            ASSERT_EQ(data_size, tree_data.size);
            ASSERT_EQ(0, memcmp(data, tree_data.buffer, data_size));
        }
        // 释放资源
        sqlite3_finalize(stmt);


    }
}
