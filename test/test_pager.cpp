//
// Created by user on 25-2-13.
//
#include "test.h"
#include "os.h"

namespace iedb
{
    void test::test_pager(const std::string& path)
    {
        static constexpr int size = 1024 * 1024 * 1024;
        static constexpr int page_count = size / page_size;
        //确保该文件不存在
        if (os::access(path.c_str(), os::access_mode_file_exists) == status_ok)
            os::unlink(path.c_str());
        //开启新文件
        auto p = pager::open(path);
        p->rollback_back();
        /*
         *首先测试连续写入，即生成指定大小文件
         */
        pager::dbPage_ref ref;
        p->begin_write_transaction();
        //文件写入
        for (auto i = 0; i < page_count; ++i)
        {
            p->get_new_page( ref);
            auto& page = ref.value().get();
            page.enable_write();
            auto value = static_cast<int64*>(page.get_data());
            *value = i;
        }
        p->commit_phase_one();
        p->commit_phase_two();
        p->release_buffer();
        //测试能否正常读取写入数据
        for (auto i = 0; i < page_count; ++i)
        {
            p->get_page(i, ref);
            auto& page = ref.value().get();
            auto value = static_cast<int64*>(page.get_data());
            assert(*value == i);
        }
        p->release_buffer();
        //测试随机写入
        //获取随机数
        int pages_no[page_count];
        test::get_random(pages_no, sizeof(pages_no));
        //对随机页面进行写入
        p->begin_write_transaction();
        for (auto i = 0; i < page_count; ++i)
        {
            if (pages_no[i] < 0 )
                pages_no[i] = -pages_no[i];
            auto no = pages_no[i] % page_count;
            assert(p->get_page(no, ref) == status_ok);
            auto& page = ref.value().get();
            page.enable_write();
            auto value = static_cast<int64*>(page.get_data());
            *value = no + 1;
        }
        p->commit_phase_one();
        p->commit_phase_two();
        p->release_buffer();
        //测试能否正常读取写入数据
        for (auto i = 0; i < page_count; ++i)
        {
            if (pages_no[i] < 0 )
                pages_no[i] = -pages_no[i];
            auto no = pages_no[i] % page_count;
            p->get_page(no, ref);
            auto& page = ref.value().get();
            auto value = static_cast<int64*>(page.get_data());
            assert(*value == no + 1);
        }
        p->release_buffer();


        printf("wow! the pager running all sample successfully!");
    }
}
