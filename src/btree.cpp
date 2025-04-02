//
// Created by user on 25-2-19.
//
#include "../include/btree.h"
#define CHECK_ERROR(x) {_status = x;assert(_status == status_ok);if (_status != status_ok) return _status;}

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
    constexpr btree_internal_page* btree::open_internal_page(const dbPage_ref& page_ref)
    {
        return btree_internal_page::open(page_ref->get().get_data());
    }
    constexpr btree_leaf_page* btree::open_leaf_page(const dbPage_ref& page_ref)
    {
        return btree_leaf_page::open(page_ref->get().get_data());
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
        auto new_page = btree_leaf_page::init(new_page_buff, page_ref->get().get_page_no(), page->next_page);
        page->next_page = new_page_ref->get().get_page_no();
        page_ref->get().enable_write();
        //对新页进行平衡
        uint64 middle_key = 0;
        btree_leaf_page::balance(page, new_page, middle_key);
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
        btree_internal_page::split(ori_page, new_page_buff, ori_page_no, new_page_no, middle_key);
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
        uint64 front_key, back_key;
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
            CHECK_ERROR(
                insert_to_full_internal_page(page_ref,back_key, front_no, back_no, back_key, front_no, back_no));
        }
        //连最顶层页面都进行分裂，则需申请新页并修改root页面
        dbPage_ref new_page_ref;
        CHECK_ERROR(allocate_page(new_page_ref));
        auto root_page = btree_internal_page::init(new_page_ref->get().get_data(), 0, 0);
        header.root_page_no = new_page_ref->get().get_page_no();
        assert(root_page->insert(back_key,front_no,back_no) == status_ok);
        return status_ok;
    }

    int btree::obtain_nearby_page(const dbPage_ref& upper_page_ref, int current_page_no, dbPage_ref& out_front_Page_ref, dbPage_ref& out_back_page_ref) const
    {
        auto _status = 0;
        auto page = open_internal_page(upper_page_ref);
        auto key_count = page->key_count;
        auto front_page_no = 0;
        auto back_page_no = 0;
        auto found = false;
        //确定current_page_no在内部页中的位置
        for (auto i = 0;i < key_count;i++)
        {
            if (page->pages_no[i] == current_page_no)
            {
                front_page_no = current_page_no;
                back_page_no = page->pages_no[i + 1];
                found = true;
                break;
            }
        }
        if (found == false)
        {
            if (page->pages_no[key_count] != current_page_no)
                throw std::runtime_error("upper_page中应当存在current_page_no,但并未查找到");
            //为避免跨页，必须向前查找
            front_page_no = page->pages_no[key_count - 1];
            back_page_no = current_page_no;
        }
        CHECK_ERROR(_pager->get_page(front_page_no,out_front_Page_ref));
        CHECK_ERROR(_pager->get_page(back_page_no,out_back_page_ref));
        return status_ok;
    }

    int btree::adjust_nearby_leaf_page(const dbPage_ref& front_page_ref, const dbPage_ref& back_page_ref, bool& out_is_merge, uint64& out_key, int& out_next_page_no_for_front)
    {
        auto _status = 0;
        CHECK_ERROR(front_page_ref->get().enable_write());
        CHECK_ERROR(back_page_ref->get().enable_write());
        auto front_page = open_leaf_page(front_page_ref);
        auto back_page = open_leaf_page(back_page_ref);
        auto front_size = front_page->payload_size_count;
        auto back_size = back_page->payload_size_count;
        auto front_count = front_page->payload_count;
        auto back_count = back_page->payload_count;
        //两者总payload大小小于一定值才可进行合并，否则仅能进行平衡
        if (front_size + back_size < page_size / 4 * 3)
        {
            out_is_merge = true;
            // 前后两页进行合并
            assert(btree_leaf_page::merge(front_page, back_page) == status_ok);
            assert(front_size + back_size == front_page->payload_size_count);
            assert(front_count + back_count == front_page->payload_count);
            out_key = front_page->payloads[front_page->payload_count - 1].key;
        }
        else
        {
            out_is_merge = false;
            //需要对前后两页进行平衡
            btree_leaf_page::balance(front_page, back_page, out_key);
        }
        out_next_page_no_for_front = front_page->next_page;
        return status_ok;
    }

    int btree::adjust_nearby_internal_page(const dbPage_ref& upper_page_ref, const dbPage_ref& front_page_ref, const dbPage_ref& back_page_ref,bool & out_is_merge)
    {
        auto _status = 0;
        auto key = 0UL;
        auto front_page_no = front_page_ref->get().get_page_no();
        auto back_page_no = back_page_ref->get().get_page_no();
        auto upper_page = open_internal_page(upper_page_ref);
        auto front_page = open_internal_page(front_page_ref);
        auto back_page = open_internal_page(back_page_ref);
        assert(front_page->next_page == back_page_no);
        assert(back_page->prev_page == front_page_no);
        assert(upper_page->search(front_page_no,back_page_no,key) == status_ok);
        //确保三者能够被写入
        CHECK_ERROR(upper_page_ref->get().enable_write());
        CHECK_ERROR(front_page_ref->get().enable_write());
        CHECK_ERROR(back_page_ref->get().enable_write());
        //判断进行合并还是平衡
        if (front_page->key_count + back_page->key_count + 1 < btree_internal_page::key_capacity)
        {
            //优先进行合并
            out_is_merge = true;
            assert(btree_internal_page::merge(front_page,back_page,key) == status_ok);
            upper_page->remove(back_page_no) ;
            return status_ok;
        }
        //否则进行平衡
        out_is_merge = false;
        auto middle_key = key;
        btree_internal_page::balance( front_page, back_page, middle_key,key);
        assert(upper_page->update(key,front_page_no,back_page_no) == status_ok);
        return status_ok;
    }


    int btree::adjust_tree(uint64 key)
    {
        assert(_status == status::write);
        std::stack<int> stack;
        dbPage_ref upper_page_ref,front_page_ref,back_page_ref;
        auto _status = 0;
        CHECK_ERROR(search_page(key, stack));
        assert(stack.empty() == false);
        //首先确定是否需要调整
        auto current_page_no = stack.top();
        stack.pop();
        //如果当前页面为根页则无需任何操作
        if (current_page_no == header.root_page_no)
            return status_ok;
        assert(stack.empty() == false);
        //获取上一级页面
        auto upper_page_no = stack.top();
        stack.pop();
        CHECK_ERROR(_pager->get_page(upper_page_no,upper_page_ref));
        auto merge_flag = false;
        CHECK_ERROR(obtain_nearby_page(upper_page_ref,current_page_no,front_page_ref,back_page_ref));
        auto front_page_no = front_page_ref->get().get_page_no();
        auto back_page_no = back_page_ref->get().get_page_no();
        auto next_page_no_for_front = 0;
        CHECK_ERROR(adjust_nearby_leaf_page(front_page_ref,back_page_ref,merge_flag,key,next_page_no_for_front));
        //如果仅进行平衡，则在修改上一层页面对应项目后返回
        if (merge_flag == false)
        {
            auto internal_page = open_internal_page(upper_page_ref);
            CHECK_ERROR(upper_page_ref->get().enable_write());
            assert(internal_page->update(key, front_page_no, back_page_no) == status_ok);
            return status_ok;
        }
        //否则需要先释放back_page后删除对应项目
        CHECK_ERROR(release_page(back_page_ref));
        auto upper_page = open_internal_page(upper_page_ref);
        //然后修改合并页面对应项目
        upper_page->remove(back_page_no);
        // upper_page->update(key + 1, front_page_no, next_page_no_for_front);
        while (stack.empty() == false)
        {
            // 首先判断upper_page是否需要进行进一步操作
            if (upper_page->key_count >= btree_internal_page::key_capacity / 4)
                return status_ok;
            //开始进一步操作
            //首先获取各个页面
            current_page_no = upper_page_no;
            upper_page_no = stack.top();
            stack.pop();
            CHECK_ERROR(_pager->get_page(upper_page_no, upper_page_ref));
            upper_page_no = upper_page_ref->get().get_page_no();
            CHECK_ERROR(obtain_nearby_page(upper_page_ref,current_page_no,front_page_ref,back_page_ref));
            //开始处理
            CHECK_ERROR(adjust_nearby_internal_page(upper_page_ref,front_page_ref,back_page_ref,merge_flag));
            //如果未进行合并则可直接返回
            if (merge_flag == false)
                return status_ok;
            // 否则需要先释放back_page
            CHECK_ERROR(release_page(back_page_ref));
        }
        //如果upper_page到达根页仍未返回
        //考虑根页内不存在key，需更换根页
        if (upper_page->key_count == 0)
        {
            header.root_page_no = upper_page->pages_no[0];
            CHECK_ERROR(release_page(upper_page_ref));
        }
        //其他情况下可直接返回
        return status_ok;
    }

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
        out_cursor = std::make_unique<cursor>(*this, page_ref, _cursor);
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
