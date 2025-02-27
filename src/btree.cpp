//
// Created by user on 25-2-19.
//
#include "../include/btree.h"
#define CHECK_ERROR(x) {_status = x;if (_status != status_ok) return _status;}

namespace iedb
{
    void btree::btree_header::init(void* buffer, int root_page_no, int page_count, int free_page_no)
    {
        auto header = static_cast<btree_header*>(buffer);
        header->root_page_no = root_page_no;
        header->page_count = page_count;
        header->free_page_no = free_page_no;
        memcpy(header->magic, btree_header::magic_string, sizeof(btree_header::magic_string));
    }

    btree::btree(std::unique_ptr<pager>& _pager, const btree_header& header): _status(status::normal),
                                                                              _pager(std::move(_pager)), header(header),
                                                                              ref_count(0)
    {
    }

    constexpr btree_page_type btree::get_page_type(const dbPage_ref& page)
    {
        return *static_cast<btree_page_type*>(page->get().get_data());
    }


    std::unique_ptr<btree> btree::open(std::unique_ptr<pager>& _pager)
    {
        pager::dbPage_ref ref;
        auto status = _pager->get_page(0, ref);
        if (status != status_ok)
        {
            fprintf(stderr, "can't get the no.0 page when open btree\n");
            return nullptr;
        }
        auto& page = ref->get();
        auto header = static_cast<btree_header*>(page.get_data());
        //如果魔数不匹配，则说明需进行初始化
        if (strcmp(header->magic, btree_header::magic_string) != 0)
        {
            //需要两个页面，0号页保存元数据，新页作为根页
            dbPage_ref new_page_ref;
            _pager->begin_write_transaction();
            _pager->get_new_page(new_page_ref);
            auto& new_page = new_page_ref->get();
            //将两个页面均标记为可写
            page.enable_write();
            new_page.enable_write();
            //分别进行初始化
            auto new_page_no = new_page.get_page_no();
            btree_header::init(page.get_data(), new_page_no, 1, 0);
            btree_leaf_page::init(new_page.get_data(), 0, 0);
            _pager->commit_phase_one();
            _pager->commit_phase_two();
        }
        return std::unique_ptr<btree>(new btree(_pager, *header));
    }


    int btree::enable_write()
    {
        assert(_status == status::normal);
        auto _status = 0;
        CHECK_ERROR(_pager->begin_write_transaction());
        this->_status = status::write;
        return status_ok;
    }

    int btree::commit()
    {
        assert(_status == status::write);
        //获取0号页，将header写入0号页后提交
        pager::dbPage_ref page_ref;
        auto _status = 0;
        CHECK_ERROR(_pager->get_page(0,page_ref));
        auto& page = page_ref->get();
        page.enable_write();
        memcpy(page.get_data(), &header, sizeof(header));
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
        }
        else
        {
            CHECK_ERROR(_pager->get_new_page(ref));
        }
        return ref->get().enable_write();
    }

    int btree::release_page(const pager::dbPage_ref& ref)
    {
        assert(_status == status::write);
        ref->get().enable_write();
        auto page_header = static_cast<btree_leaf_page*>(ref->get().get_data());
        page_header->type = btree_page_type::free;
        page_header->next_page = header.free_page_no;
        header.free_page_no = ref->get().get_page_no();
        return status_ok;
    }


    int btree::search_page(uint64 key, std::stack<int>& page_no_stack) const
    {
        assert(page_no_stack.empty());
        dbPage_ref page_ref;
        auto page_no = header.root_page_no;
        auto _status = 0;
        CHECK_ERROR(_pager->get_page(page_no,page_ref));
        auto type = get_page_type(page_ref);
        //如果当前页为内部页，则遍历该页获取下一级页面页号
        while (type == btree_page_type::internal)
        {
            assert(page_no > 0 && page_no < _pager->get_page_count());
            page_no_stack.push(page_no);
            auto page = btree_internal_page::open(page_ref->get().get_data());
            page->search(key, page_no);
            CHECK_ERROR(_pager->get_page(page_no,page_ref));
            type = get_page_type(page_ref);
        }
        assert(page_no > 0 && page_no < _pager->get_page_count());
        assert(type == btree_page_type::leaf);
        page_no_stack.push(page_no);
        return status_ok;
    }
    int btree::insert_to_full_leaf_page(dbPage_ref& page_ref, uint64 key, const memory_slice& data,
        int& out_back_page_no, uint64& out_back_first_key)
    {
        //获取新页
        dbPage_ref new_page_ref;
        auto _status = 0;
        auto page = btree_leaf_page::open(page_ref->get().get_data());
        CHECK_ERROR(allocate_page(new_page_ref));
        auto new_page_buff = new_page_ref->get().get_data();
        auto new_page = btree_leaf_page::init(new_page_buff,page_ref->get().get_page_no(),page->next_page);
        page->next_page = new_page_ref->get().get_page_no();
        page_ref->get().enable_write();
        //对新页进行平衡
        uint64 middle_key = 0;
        btree_leaf_page::balance(page,new_page,middle_key);
        //插入数据
        auto target_page = page;
        if (key > middle_key)
            target_page = new_page;
        auto cursor = target_page->get_cursor();
        assert(cursor.insert_payload(key,data) == status_ok);
        out_back_page_no = new_page_ref->get().get_page_no();
        out_back_first_key = new_page->payloads[0].key;
        return status_ok;
    }


    int btree::insert_to_full_internal_page(dbPage_ref& page, uint64 key,
                                            int front_page_no, int back_page_no,
                                            uint64& out_key, int& out_front_page_no, int& out_back_page_no)
    {
        dbPage_ref new_page_ref;
        dbPage_ref ori_page_ref = page;
        auto _status = 0;
        //先获取需要进行处理的两个页面及其它们的页号,然后标记它们为可写
        CHECK_ERROR(allocate_page(new_page_ref));
        ori_page_ref->get().enable_write();
        new_page_ref->get().enable_write();
        auto ori_page = btree_internal_page::open(ori_page_ref->get().get_data());
        auto ori_page_no = ori_page_ref->get().get_page_no();
        auto new_page_no = new_page_ref->get().get_page_no();
        auto new_page_buff = new_page_ref->get().get_data();
        //对页面进行分裂
        uint64 middle_key = 0;
        btree_internal_page::split(ori_page,new_page_buff,ori_page_no,new_page_no,middle_key);
        //插入数据
        auto new_page = btree_internal_page::open(new_page_buff);
        if (key > middle_key)
            assert(new_page->insert(key,front_page_no,back_page_no) == status_ok);
        else
            assert(ori_page->insert(key,front_page_no,back_page_no) == status_ok);

        out_key = middle_key;
        out_front_page_no = ori_page_no;
        out_back_page_no = new_page_no;
        return status_ok;
    }


    int btree::insert(uint64 key, const memory_slice& data)
    {
        assert(_status == status::write);
        std::stack<int> stack;
        dbPage_ref page_ref;
        auto _status = 0;
        CHECK_ERROR(search_page(key, stack));
        //如果返回status_ok，则说明插入值非当前最小值，按正常逻辑处理
        //向栈顶页面插入数据
        memory_slice current_slice = data;
        uint64  front_key,back_key;
        int back_no;
        assert(stack.empty() == false);
        //首先考虑仅需插入叶节点的情况
        auto current_page_no = stack.top();
        stack.pop();
        CHECK_ERROR(_pager->get_page(current_page_no,page_ref));
        CHECK_ERROR(page_ref->get().enable_write());
        auto page = btree_leaf_page::open(page_ref->get().get_data());
        auto cursor = page->get_cursor();
        _status = cursor.insert_payload(key, current_slice);
        if (_status == status_ok)
            return status_ok;
        //如果空间不足，则需要进行分裂
        auto front_no = page_ref->get().get_page_no();
        CHECK_ERROR(insert_to_full_leaf_page(page_ref,key,current_slice,back_no,back_key));
        while (stack.empty() == false)
        {
            //从此处开始，将key与页号相关信息插入内部页
            current_page_no = stack.top();
            stack.pop();
            CHECK_ERROR(_pager->get_page(current_page_no,page_ref));
            CHECK_ERROR(page_ref->get().enable_write());
            auto internal_page = btree_internal_page::open(page_ref->get().get_data());
            _status = internal_page->insert(back_key, front_no, back_no);
            if (_status == status_ok)
                return status_ok;
            //当前插入内部页空间不足，需要分裂并将结果插入上一层页
            CHECK_ERROR(insert_to_full_internal_page(page_ref,back_key, front_no, back_no, back_key, front_no, back_no));
        }
        //连最顶层页面都进行分裂，则需申请新页并修改root页面
        dbPage_ref new_page_ref;
        CHECK_ERROR(allocate_page(new_page_ref));
        auto root_page = btree_internal_page::init(new_page_ref->get().get_data(), 0, 0);
        header.root_page_no = new_page_ref->get().get_page_no();
        assert(root_page->insert(back_key,front_no,back_no) == status_ok);
        return status_ok;
    }


    // int btree::merge_page(dbPage_ref& merge_page_ref, dbPage_ref& merged_page_ref, uint64& out_merged_page_first_key)
    // {
    //     auto front_page = open_page(merge_page_ref);
    //     auto back_page = open_page(merged_page_ref);
    //     assert(front_page->payloads[0].key < back_page->payloads[0].key);
    //     assert(front_page->next_page == merged_page_ref->get().get_page_no());
    //     auto _status = 0;
    //     //首先判断是否能够进行合并
    //     auto payload_size_count = front_page->payload_size_count + back_page->payload_size_count;
    //     auto payload_count = front_page->payload_count + back_page->payload_count;
    //     auto merged_data_size_count = sizeof(btree_leaf_page) + payload_count * sizeof(btree_leaf_page::payload_meta) +
    //         payload_size_count;
    //     if (merged_data_size_count >= page_size)
    //         return status_no_space;
    //     //开始准备合并
    //     CHECK_ERROR(merge_page_ref->get().enable_write());
    //     CHECK_ERROR(merged_page_ref->get().enable_write());
    //     out_merged_page_first_key = back_page->payloads[0].key;
    //     assert(btree_leaf_page::merge(front_page,back_page) == status_ok);
    //     return status_ok;
    // }
    //
    //
    //
    // int btree::delete_item(uint64 key)
    // {
    //     assert(_status == status::write);
    //     std::stack<int> stack;
    //     dbPage_ref page_ref;
    //     uint64 current_key;
    //     memory_slice slice{};
    //     auto _status = 0;
    //     auto page_no = 0;
    //     auto merged_page_no = 0;
    //     //由于存在最顶层仅单个节点，删除后原二层变为顶层的情况，故需记录上一级节点页号
    //     auto last_merged_page = 0;
    //     CHECK_ERROR(search_page(key,stack));
    //     while (stack.empty() == false)
    //     {
    //         page_no = stack.top();
    //         uint64 deleted_page_first_key;
    //         stack.pop();
    //         CHECK_ERROR(delete_in_leaf_page(page_no,key,merged_page_no,deleted_page_first_key));
    //         //如果出现合并，则需要进一步对上一层的节点进行删除
    //         //否则直接返回
    //         if (merged_page_no == 0)
    //             return status_ok;
    //         key = deleted_page_first_key;
    //         last_merged_page = merged_page_no;
    //     }
    //     //检查根页是否还存在节点
    //     CHECK_ERROR(_pager->get_page(header.root_page_no,page_ref));
    //     auto page = btree_leaf_page::open(page_ref->get().get_data());
    //     //如果不存在节点则需释放根页，并将根页号切换为上一级的合并页页号
    //     if (page->payload_size_count == 0)
    //     {
    //         CHECK_ERROR(release_page(page_ref));
    //         header.root_page_no = last_merged_page;
    //     }
    //     return status_ok;
    // }

    int btree::get_cursor(uint64 key, std::unique_ptr<cursor>& out_cursor)
    {
        std::stack<int> stack;
        auto _status = 0;
        CHECK_ERROR(search_page(key,stack));
        auto page_no = stack.top();
        dbPage_ref page_ref;
        CHECK_ERROR(_pager->get_page(page_no,page_ref));
        auto page = btree_leaf_page::open(page_ref->get().get_data());
        auto page_cursor = page->get_cursor();
        CHECK_ERROR(page_cursor.search_payload_first_le(key));
        out_cursor = std::make_unique<cursor>(*this, page_ref, page_cursor);
        return status_ok;
    }

    int btree::get_first_cursor(std::unique_ptr<cursor>& out_cursor)
    {
        //从根页开始，不断获取第一个payload
        auto page_no = header.root_page_no;
        dbPage_ref page_ref;
        auto _status = 0;
        CHECK_ERROR(_pager->get_page(page_no,page_ref));
        auto type = get_page_type(page_ref);
        while (type == btree_page_type::internal)
        {
            auto page = btree_internal_page::open(page_ref->get().get_data());
            assert(page->key_count);
            page_no = page->pages_no[0];
            CHECK_ERROR(_pager->get_page(page_no,page_ref));
            type = get_page_type(page_ref);
        }
        auto page = btree_leaf_page::open(page_ref->get().get_data());
        auto _cursor = page->get_cursor();
        CHECK_ERROR(_cursor.first());
        out_cursor = std::make_unique<cursor>(*this, page_ref,_cursor);
        return status_ok;
    }

    int btree::get_last_cursor(std::unique_ptr<cursor>& out_cursor)
    {
        //从根页开始，不断获取最后一个payload
        auto page_no = header.root_page_no;
        dbPage_ref page_ref;
        auto _status = 0;
        CHECK_ERROR(_pager->get_page(page_no,page_ref));
        auto page = btree_leaf_page::open(page_ref->get().get_data());
        uint64 key;
        memory_slice slice{};
        auto _cursor = page->get_cursor();
        CHECK_ERROR(_cursor.last());
        while (page->type == btree_page_type::internal)
        {
            _cursor.get_payload(key, slice);
            page_no = *static_cast<int*>(slice.buffer);
            CHECK_ERROR(_pager->get_page(page_no,page_ref));
            page = btree_leaf_page::open(page_ref->get().get_data());
            _cursor = page->get_cursor();
            CHECK_ERROR(_cursor.last());
        }
        out_cursor = std::make_unique<cursor>(*this, page_ref, _cursor);
        return status_ok;
    }
}
