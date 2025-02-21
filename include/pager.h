//
// Created by user on 25-2-12.
//

#ifndef PAGER_H
#define PAGER_H


#include "journal.h"
/*
 * pager仅保证数据文件写入的原子性且每次写入以页为单位
 *
 */
namespace iedb
{
    class pager
    {
    public:
        static constexpr int max_cache_page = 4 * 1024;
        class dbPage
        {
        private:
            pager& owner;
            int page_no;
            bool writable;
            std::unique_ptr<std::array<uint8,page_size>> data;
        public:
            friend class pager;
            ~dbPage() = default;
            dbPage(dbPage&) = delete;
            dbPage(pager& owner,int page_no);
            [[nodiscard]] int get_page_no() const;
            [[nodiscard]]void* get_data() const;
            /*
             * 在对页面进行修改前调用该函数
             */
            int enable_write();
        };
    private:
        enum class pager_status
        {
            normal,
            write_transaction,
            commit,
            error
        };
        std::unique_ptr<journal> j;
        int fd;
        int64 page_count;
        pager_status status;
        //用于缓存页面并通过页号获取缓存的页面
        std::list<dbPage> pages;
        std::vector<dbPage*> writable_pages;
        std::unordered_map<int, dbPage> map;

        int mark_page_writable(dbPage& page);
        pager(int fd,std::unique_ptr<journal>&j,int64 original_file_size);
    public:
        ~pager();
        using dbPage_ref = std::optional<std::reference_wrapper<dbPage>>;
        pager(pager&) = delete;
        static std::unique_ptr<pager> open(const std::string& path);
        //用于数据页面管理
        int get_page(int page_no,dbPage_ref& out_page);
        int get_new_page(dbPage_ref& out_page);
        void release_buffer();
        //用于写事务的管理
        int begin_write_transaction();
        int commit_phase_one();
        int commit_phase_two();
        int rollback_back();
    };
using dbPage_ref = pager::dbPage_ref;



}


#endif //PAGER_H
