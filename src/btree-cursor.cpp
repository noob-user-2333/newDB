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
    btree::cursor::cursor(btree& owner, dbPage_ref& page_ref,
        const btree_page::btree_cursor& page_cursor):owner(owner),page_ref(page_ref),page_cursor(page_cursor)
    {}

    int btree::cursor::enable_write()
    {
        return owner.enable_write();
    }
    int btree::cursor::commit()
    {
        return owner.commit();
    }

    int btree::cursor::delete_item()
    {
        assert(owner._status == status::write);
        auto page= get_btree_page(page_ref);
        page_ref->get().enable_write();
        uint64 key;
        memory_slice unused{};
        page_cursor.get_payload(key,unused);
        return owner.delete_item(key);
    }
    void btree::cursor::get_item(uint64&out_key,memory_slice& out_data) const
    {
        page_cursor.get_payload(out_key,out_data);
    }

    int btree::cursor::update_item(const memory_slice& data)
    {
        assert(owner._status == status::write);
        memory_slice slice{};
        uint64 key;
        auto _status = 0;
        get_item(key, slice);
        CHECK_ERROR(delete_item());
        CHECK_ERROR(owner.insert(key,data));
        return status_ok;
    }

    int btree::cursor::next()
    {
        auto _status = page_cursor.next();
        if (_status == status_ok)
            return status_ok;
        auto page = get_btree_page(page_ref);
        //否则需要切换到下一页
        //如果next_page为0，则说明已经到达末尾
        if (page->next_page == 0)
            return status_out_of_range;
        CHECK_ERROR(owner._pager->get_page(page->next_page,page_ref));
        page = get_btree_page(page_ref);
        assert(page->payload_count);
        page_cursor = page->get_cursor();
        return page_cursor.next();

    }
int btree::cursor::prev()
    {
        auto _status = page_cursor.previous();
        if (_status == status_ok)
            return status_ok;
        auto page = get_btree_page(page_ref);
        //否则需要切换到下一页
        //如果prev_page为0，则说明已经到达头部
        if (page->prev_page == 0)
            return status_out_of_range;
        CHECK_ERROR(owner._pager->get_page(page->prev_page,page_ref));
        page = get_btree_page(page_ref);
        assert(page->payload_count);
        page_cursor = page->get_cursor();
        return page_cursor.last();
    }


}