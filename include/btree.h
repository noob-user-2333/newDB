//
// Created by user on 25-2-14.
//

#ifndef BTREE_H
#define BTREE_H
#include "pager.h"
#include "btree_page.h"

namespace iedb
{
    /*
     *  给定键值，获取cursor，生成最接进
     *
     *
     *
     */

    class btree
    {
    public:

        class cursor
        {
        private:
            btree& owner;
            dbPage_ref page_ref;
            int index;
            static btree_page* get_btree_page(dbPage_ref& page_ref);
        public:
            cursor(cursor&) = delete;
            cursor(btree& owner,btree_page * page,int index);
            int enable_write();
            int commit();
            int insert_item(uint64 key,const memory_slice & data) const;
            // int delete_item();
            // int update_item();
            // int next();
            // int prev();

        };
    private:
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
        };
        struct free_pager_header
        {
            btree_page_type type;
            int next_page;
        };
        status _status;
        std::unique_ptr<pager> _pager;
        btree_header header;
        int ref_count;
/*
 *  该部分负责以页为单位的Btree操作
 */
        static int init(std::unique_ptr<pager> & _pager);
        int enable_write();
        int commit();
        int allocate_page(pager::dbPage_ref & ref);
        int release_page(const pager::dbPage_ref & ref);




        // static int internal_page_search(btree_page * page,uint64 key,int &out_next_page);

        //获取对查询条件为 >=key 的情况，第一个符合条件的payload所在的页面
        // int search_page(uint64 key,std::stack<int>& page_no_stack) const;
        //当原页空间不足时调用，将原页分裂后插入数据并输出新页页号
        // int insert_to_full_page(dbPage_ref& page,uint key,const memory_slice & data,int & out_new_page_no,uint64 & out_new_middle_key);
        // int delete_item(uint64 key);
        btree(std::unique_ptr<pager>& _pager, const btree_header&header);

    public:
        btree(const btree&) = delete;
        btree(btree&&) = delete;
        ~btree() = default;
        static std::unique_ptr<btree> open(std::unique_ptr<pager>&_pager);
        cursor get_cursor();
    };




}

#endif //BTREE_H
