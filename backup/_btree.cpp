//
// Created by user on 25-2-19.
//
#include "../include/btree.h"
#define CHECK_ERROR(x) {_status = x;if (_status != status_ok) return _status;}
namespace iedb
{
    btree::btree(std::unique_ptr<pager>& _pager, const btree_header&header):_status(status::normal),_pager(std::move(_pager)),header(header),ref_count(0)
    {
    }
    std::unique_ptr<btree> btree::open(std::unique_ptr<pager>& _pager)
    {
        pager::dbPage_ref ref;
        auto status = _pager->get_page(0,ref);
        if (status!= status_ok)
        {
            fprintf(stderr,"can't get the no.0 page when open btree\n");
            return nullptr;
        }
        auto& page = ref->get();
        auto header = static_cast<btree_header*>(page.get_data());
        //如果魔数不匹配，则说明需进行初始化
        if (strcmp(header->magic,btree_header::magic_string) != 0)
        {
            _pager->begin_write_transaction();
            page.enable_write();
            btree_page::init(page.get_data(),btree_page_type::leaf,0,0);
            _pager->commit_phase_one();
            _pager->commit_phase_two();
        }
        return std::unique_ptr<btree>(new btree(_pager, *header));
    }

    /*
     * 对_pager的0号页面进行初始化，使其作为btree根页
     */
    int btree::init(std::unique_ptr<pager>& _pager)
    {
        auto _status = _pager->begin_write_transaction();
        //获取0号页
        pager::dbPage_ref page_ref;
        CHECK_ERROR(_pager->get_page(0,page_ref));
        auto& page = page_ref->get();
        CHECK_ERROR(page.enable_write());
        //在0号页面中创建一个新的btree根页
        auto page_data = page_ref->get().get_data();
        auto root_page = btree_page::init(page_data, btree_page_type::leaf,0,0);
        //将修改提交
        CHECK_ERROR(_pager->commit_phase_one());
        CHECK_ERROR(_pager->commit_phase_two());
        return status_ok;
    }
    int btree::enable_write()
    {
        assert(_status == status::normal);
        auto _status = 0;
        CHECK_ERROR(_pager->begin_write_transaction());
        this->_status = status::write;
        return  status_ok;
    }

    int btree::commit()
    {
        assert(_status == status::write);
        //获取0号页，将header写入0号页后提交
        pager::dbPage_ref page_ref;
        auto _status = 0;
        CHECK_ERROR(_pager->get_page(0,page_ref));
        auto& page = page_ref->get();
        auto page_data = page.get_data();
        page.enable_write();
        memcpy(page.get_data(),&header,sizeof(header));
        //将修改提交
        CHECK_ERROR(_pager->commit_phase_one());
        CHECK_ERROR(_pager->commit_phase_two());
        this->_status = status::normal;
        return status_ok;
    }

    int btree::allocate_page(pager::dbPage_ref& ref)
    {
        assert(_status == status::write);
        auto _status = 0;
        //优先使用空闲链表中的页面
        if (header.free_page_no)
        {
            CHECK_ERROR(_pager->get_page(header.free_page_no,ref));
            auto page_header = static_cast<free_pager_header*>(ref->get().get_data());
            assert(page_header->type == btree_page_type::free);
            header.free_page_no = page_header->next_page;
        }else
        {
            CHECK_ERROR(_pager->get_new_page(ref));
        }
        return ref->get().enable_write();
    }
    int btree::release_page(const pager::dbPage_ref& ref)
    {
        assert(_status == status::write);
        ref->get().enable_write();
        auto page_header = static_cast<btree_page*>(ref->get().get_data());
        page_header->type = btree_page_type::free;
        page_header->next_page = header.free_page_no;
        header.free_page_no = ref->get().get_page_no();
        return status_ok;
    }

    int btree::internal_page_search(btree_page* page, uint64 key,int& out_next_page)
    {
        assert(page);
        auto cursor = page->get_cursor();
        auto _status = 0;
        CHECK_ERROR(cursor.search_payload_more_equal(key));
        //如果找到了payload
        memory_slice data{};
        cursor.get_payload(key, data);
        //获取下一个页面页号
        out_next_page = *static_cast<int*>(data.buffer);
        return status_ok;
    }



    int btree::search_page(uint64 key, std::stack<int>& page_no_stack) const
    {
        //清空stack
        while (!page_no_stack.empty())
            page_no_stack.pop();
        dbPage_ref page_ref;
        auto page_no = header.root_page_no;
        auto _status = 0;
        CHECK_ERROR(_pager->get_page(page_no,page_ref));
        auto page_data = page_ref->get().get_data();
        auto page = btree_page::open(page_data);
        //如果当前页为内部页，则遍历该页获取下一级页面页号
        while (page->type == btree_page_type::internal)
        {
            page_no_stack.push(page_no);
            CHECK_ERROR(internal_page_search(page,key,page_no));
            page_data = page_ref->get().get_data();
            page = btree_page::open(page_data);
        }
        page_no_stack.push(page_no);
        return status_ok;
    }

    int btree::insert_to_full_page(dbPage_ref& page, uint key, const memory_slice& data, int& out_new_page_no, uint64& out_new_middle_key)
    {
        dbPage_ref _next_page;
        auto _status = 0;
        //先获取需要进行处理的两个页面及其它们的页号,然后标记它们为可写
        auto front_page_no = page->get().get_page_no();
        CHECK_ERROR(allocate_page(_next_page));
        auto next_page_no = _next_page->get().get_page_no();
        page->get().enable_write();
        _next_page->get().enable_write();
        //对新生成的页进行初始化,将其挂载到页面链表中
        auto front_page = btree_page::open(page->get().get_data());
        auto page_type = front_page->type;
        auto page_next_page_no = front_page->next_page;
        front_page->next_page = next_page_no;
        auto next_page = btree_page::init(_next_page->get().get_data(),page_type,front_page_no,page_next_page_no);
        out_new_page_no = next_page_no;
        //对两者页面数据进行平衡
        uint64 middle_key = 0;
        btree_page::balance(front_page,next_page,middle_key);
        //确定新数据应当插入哪个页面
        if (key >= middle_key)
        {
            auto cursor = next_page->get_cursor();
            CHECK_ERROR(cursor.insert_payload(key,data));
        }else
        {
            auto cursor = front_page->get_cursor();
            CHECK_ERROR(cursor.insert_payload(key,data));
        }
        out_new_middle_key = middle_key;
        return status_ok;
    }

    int btree::insert(uint64 key, const memory_slice& data)
    {
        assert(_status == status::write);
        std::stack<int> stack;
        dbPage_ref page_ref;
        uint64 buff;
        auto _status = 0;
        auto current_key = key;
        CHECK_ERROR(btree::search_page(key,stack));
        //向栈顶页面插入数据
        memory_slice current_slice = data;
        while (stack.empty() == false)
        {
            auto current_page_no = stack.top();
            stack.pop();
            CHECK_ERROR(_pager->get_page(current_page_no,page_ref));
            auto page_data = page_ref->get().get_data();
            auto page = btree_page::open(page_data);
            auto cursor = page->get_cursor();
            _status = cursor.insert_payload(current_key,current_slice);
            if (_status == status_ok)
                return status_ok;
            //如果空间不足，则需要进行分裂
            int new_page_no;
            uint64 middle_key;
            CHECK_ERROR(insert_to_full_page(page_ref,key,current_slice,new_page_no,middle_key));
            current_key = middle_key;
            buff = new_page_no;
            current_slice.set(&buff,current_key);
        }
        //连最顶层页面都进行分裂，则需申请新页并修改root页面
        CHECK_ERROR(allocate_page(page_ref));
        auto root_page = btree_page::init(page_ref->get().get_data(),btree_page_type::internal,0,0);
        header.root_page_no = page_ref->get().get_page_no();
        auto cursor = root_page->get_cursor();
        return cursor.insert_payload(current_key,current_slice);
    }
    int btree::delete_item(uint64 key)
    {
        assert(_status == status::write);
        std::stack<int> stack;
        dbPage_ref page_ref;
        uint64 current_key;
        memory_slice slice{};
        auto _status = 0;
        CHECK_ERROR(search_page(key,stack));
        while (stack.empty() == false)
        {
            auto page_no = stack.top();
            stack.pop();
            CHECK_ERROR(_pager->get_page(page_no,page_ref));
            auto page_data = page_ref->get().get_data();
            auto page = btree_page::open(page_data);
            auto cursor = page->get_cursor();
            CHECK_ERROR(cursor.search_payload_less_equal(key));
            cursor.get_payload(current_key,slice);
            if (current_key != key)
                return status_not_found;
            //如果找到了payload，则进行删除
            return cursor.delete_payload();
        }

    }


}