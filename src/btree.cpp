//
// Created by user on 25-2-21.
//
#include "btree.h"
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






}