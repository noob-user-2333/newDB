//
// Created by user on 25-2-14.
//

#ifndef BTREE_H
#define BTREE_H
#include "pager.h"
#include "btree_internal_page.h"

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
            btree_leaf_page::btree_cursor page_cursor;
            static btree_leaf_page* get_btree_page(dbPage_ref& page_ref);
        public:
            cursor(cursor&) = delete;
            cursor(btree& owner,dbPage_ref & page_ref, const btree_leaf_page::btree_cursor& page_cursor);
            int enable_write();
            int commit();
            int remove();
            void get_item(uint64& out_key,memory_slice & out_data) const;
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
    public:
        status _status;
        std::unique_ptr<pager> _pager;
        btree_header header;
        int ref_count;
/*
 *  该部分负责以页为单位的Btree操作
 */

        static constexpr btree_internal_page* open_internal_page(const dbPage_ref&page_ref);
        static constexpr btree_leaf_page* open_leaf_page(const dbPage_ref& page_ref);
        static constexpr btree_page_type get_page_type(const dbPage_ref&page);
        int allocate_page(pager::dbPage_ref & ref);
        int release_page(const pager::dbPage_ref & ref);
        int search_page(uint64 key, std::stack<int>& page_no_stack) const;

        //默认page已满，需要先生成新页保存部分数据后插入
        int insert_to_full_leaf_page(dbPage_ref& page_ref,uint64 key, const memory_slice& data,int& out_back_page_no,uint64& out_back_first_key);
        int insert_to_full_internal_page(dbPage_ref& page_ref,uint64 key,int front_page_no,int back_page_no,
            uint64 &out_key,int & out_front_page_no,int & out_back_page_no);
        int obtain_nearby_page(const dbPage_ref& upper_page_ref,int current_page_no,dbPage_ref&out_front_Page_ref,dbPage_ref&out_back_page_ref) const;
        //传入当前级与上一级页面，判断需要合并还是平衡页面，如合并则将合并后被释放的页面页号输出到out_merger_page_no
        static int adjust_nearby_leaf_page(const dbPage_ref& front_page_ref,const dbPage_ref& back_page_ref,bool & out_is_merge,uint64&out_key,int & out_next_page_no_for_front);
        static int adjust_nearby_internal_page(const dbPage_ref&upper_page_ref,const dbPage_ref& front_page_ref,const dbPage_ref& back_page_ref,bool & out_is_merge);
        //当删除数据后需要进行页面合并或平衡时使用，key仅用于确定涉及哪些页面
        int adjust_tree(uint64 key);

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
