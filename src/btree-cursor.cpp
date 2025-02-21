//
// Created by user on 25-2-19.
//
#include "../include/btree.h"
#define CHECK_ERROR(x) {_status = x;if (_status != status_ok) return _status;}
namespace iedb
{
    btree_page* btree::cursor::get_btree_page(dbPage_ref&page_ref)
    {
        return  btree_page::open(page_ref->get().get_data());
    }
    btree::cursor::cursor(btree& owner, btree_page* page, int index):owner(owner),index(index)
    {
    }
    int btree::cursor::enable_write()
    {
        return owner.enable_write();
    }
    int btree::cursor::commit()
    {
        return owner.commit();
    }
    int btree::cursor::insert_item(uint64 key, const memory_slice& data) const
    {
        assert(owner._status == status::write);
        //获取根页
        auto page_no = owner.header.root_page_no;
        dbPage_ref page_ref;
        auto _status = 0;
        CHECK_ERROR(owner._pager->get_page(page_no,page_ref));
        auto page = get_btree_page(page_ref);
        CHECK_ERROR(page_ref->get().enable_write());
        //在根页中插入数据
        auto cursor = page->get_cursor();
        _status = cursor.insert_payload(key,data);
        //如果根页已满，则需要进行分裂
        if (_status != status_ok)
        {
            //先获取新页面
            dbPage_ref new_page_ref;
            CHECK_ERROR(owner.allocate_page(new_page_ref));
            btree_page::init(new_page_ref->get().get_data(),btree_page_type::leaf,0,page_no);
            //将新页设置为根页
            owner.header.root_page_no = new_page_ref->get().get_page_no();
            page->prev_page = new_page_ref->get().get_page_no();
            auto new_page = get_btree_page(new_page_ref);
            //在新根页中插入数据
            cursor = new_page->get_cursor();
            CHECK_ERROR(cursor.insert_payload(key,data));
        }
        return status_ok;
    }






}