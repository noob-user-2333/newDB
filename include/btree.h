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
            btree& owner;
            dbPage_ref page_ref;
            btree_page::btree_cursor page_cursor;
            static btree_page* get_btree_page(dbPage_ref& page_ref);
        public:
            cursor(cursor&) = delete;
            cursor(btree& owner,dbPage_ref & page_ref, const btree_page::btree_cursor& page_cursor);
            int enable_write();
            int commit();
            int delete_item();
            void get_item(uint64& out_key,memory_slice & out_data) const;
            int update_item(const memory_slice & data);
            int next();
            int prev();

        };
        enum class status
        {
            error,
            normal,
            write
        };
        //所有btree的第0个页面仅存放btree_header
        struct btree_header
        {
            static constexpr char magic_string[16] = "iedb btree";
            char magic[16];
            int root_page_no;
            int page_count;
            int free_page_no;
            static void init(void* buffer,int root_page_no,int page_count,int free_page_no);
        };
        struct free_pager_header
        {
            btree_page_type type;
            int next_page;
        };
    private:
        status _status;
        std::unique_ptr<pager> _pager;
        btree_header header;
        int ref_count;
/*
 *  该部分负责以页为单位的Btree操作
 */
        static btree_page * open_page(const dbPage_ref & page_ref);


        int allocate_page(pager::dbPage_ref & ref);
        int release_page(const pager::dbPage_ref & ref);
        static int internal_page_search(btree_page* page, uint64 key, int& out_next_page);
        int search_page(uint64 key, std::stack<int>& page_no_stack) const;
        //默认page已满，需要先生成新页保存部分数据后插入
        int insert_to_full_page(dbPage_ref& page,const uint64 key, const memory_slice& data,
            int& out_front_page_no,uint64& out_font_first_key,
            int& out_back_page_no,uint64& out_back_first_key);
        //用于合并叶节点
        //其中merged_page表示合并过程中被释放的页面,其数据将转移到merge_page
        static int merge_page(dbPage_ref&merge_page_ref,dbPage_ref&merged_page_ref,uint64&out_merged_page_first_key);

        //该函数用于删除叶节点
        int delete_in_leaf_page(int page_no,uint64 key,int&out_merged_page_no,uint64& out_deleted_page_first_key);

        int delete_item(uint64 key);

        btree(std::unique_ptr<pager>& _pager, const btree_header&header);

    public:
        btree(const btree&) = delete;
        btree(btree&&) = delete;
        ~btree() = default;

        static std::unique_ptr<btree> open(std::unique_ptr<pager>&_pager);

        int enable_write();
        int commit();
        int insert(uint64 key, const memory_slice& data);
        int get_cursor(uint64 key,std::unique_ptr<cursor> & out_cursor);
        int get_first_cursor(std::unique_ptr<cursor> & out_cursor);
        int get_last_cursor(std::unique_ptr<cursor> & out_cursor);
    };




}

#endif //BTREE_H
