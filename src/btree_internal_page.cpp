//
// Created by user on 25-2-26.
//
#include "btree_internal_page.h"

namespace iedb
{
    btree_internal_page* btree_internal_page::init(void* data, int prev_page, int next_page)
    {
        auto p = static_cast<btree_internal_page*>(data);
        p->type = btree_page_type::internal;
        p->key_count = 0;
        p->next_page = next_page;
        p->prev_page = prev_page;
        return p;
    }
    btree_internal_page* btree_internal_page::open(void* data)
    {
        auto p = static_cast<btree_internal_page*>(data);
        assert(p->type == btree_page_type::internal);
        assert(p->key_count >= 0 && p->key_count <= key_capacity);
        return p;
    }


    int btree_internal_page::merge(btree_internal_page* front_page, btree_internal_page* back_page,uint64 middle_key)
    {
        assert(front_page->type == btree_page_type::internal && back_page->type == btree_page_type::internal);
        assert(front_page->key_count > 0 && back_page->key_count > 0);
        assert(front_page->key_count <= key_capacity && back_page->key_count <= key_capacity);
        assert(front_page->keys[front_page->key_count - 1] < middle_key);
        assert(back_page->keys[0] > middle_key);
        if (front_page->key_count + back_page->key_count + 1 > key_capacity)
            return status_no_space;
        //拷贝数据
        auto front_key_count = front_page->key_count;
        auto back_key_count = back_page->key_count;
        front_page->keys[front_key_count] = middle_key;
        front_key_count++;
        std::memcpy(front_page->keys.data()+front_key_count,back_page->keys.data(),back_key_count * sizeof(uint64));
        std::memcpy(front_page->pages_no.data()+front_key_count,back_page->pages_no.data(),(back_key_count + 1) * sizeof(int));
        front_key_count += back_key_count;
        front_page->key_count = front_key_count;
        front_page->next_page = back_page->next_page;
        return status_ok;
    }
    void btree_internal_page::split(btree_internal_page* front_page, void* back_page_buff, int front_page_no, int back_page_no, uint64& out_key)
    {
        //初始化
        auto back_page = btree_internal_page::init(back_page_buff,front_page_no,front_page->next_page);
        front_page->next_page = back_page_no;
        //平分数据
        auto split_index = front_page->key_count / 2;
        auto move_key_count = front_page->key_count - split_index - 1;
        out_key = front_page->keys[split_index];
        std::memcpy(back_page->keys.data(),front_page->keys.data() + split_index + 1,move_key_count * sizeof(uint64));
        std::memcpy(back_page->pages_no.data(),front_page->pages_no.data() + split_index + 1,(move_key_count + 1) * sizeof(int));
        back_page->key_count = move_key_count;
        front_page->key_count = split_index;
    }

    void btree_internal_page::balance(btree_internal_page* front_page, btree_internal_page* back_page, uint64 middle_key, uint64& out_new_middle_key)
    {
        assert(front_page->type == btree_page_type::internal && back_page->type == btree_page_type::internal);
        assert(front_page->key_count > 0 && back_page->key_count > 0);
        assert(front_page->key_count <= key_capacity && back_page->key_count <= key_capacity);
        assert(front_page->keys[front_page->key_count - 1] < middle_key);
        assert(back_page->keys[0] > middle_key);
        auto target_key_count = (front_page->key_count + back_page->key_count) / 2;
        //将数据先放入指定buff，之后均分至两个页面
        //提取keys
        uint64 buff[page_capacity * 2];
        std::memcpy(buff, front_page->keys.data(), front_page->key_count * sizeof(uint64));
        auto index = front_page->key_count;
        buff[index] = middle_key;
        index++;
        std::memcpy(buff + index, back_page->keys.data(), back_page->key_count * sizeof(uint64));
        index += back_page->key_count;
        //均分keys
        std::memcpy(front_page->keys.data(),buff,target_key_count * sizeof(uint64));
        out_new_middle_key = buff[target_key_count];
        std::memcpy(back_page->keys.data(),buff + target_key_count + 1,(index - target_key_count - 1) * sizeof(uint64));
        back_page->key_count = index - target_key_count - 1;
        front_page->key_count = target_key_count;
        //提取page_no
        auto no_buff = reinterpret_cast<int*>(buff);
        std::memcpy(front_page->pages_no.data(), no_buff, (front_page->key_count + 1) * sizeof(int));
        index = front_page->key_count + 1;
        std::memcpy(back_page->pages_no.data(), no_buff + index, (back_page->key_count + 1) * sizeof(int));
        index += back_page->key_count + 1;
        //均分page_no
        std::memcpy(front_page->pages_no.data(), no_buff, (front_page->key_count + 1) * sizeof(int));
        std::memcpy(back_page->pages_no.data(),no_buff + front_page->key_count + 1,(back_page->key_count + 1)*sizeof(int));
        assert(index == front_page->key_count + back_page->key_count + 2);
    }



    int btree_internal_page::insert(uint64 key, int front_page_no, int back_page_no)
    {
        if (key_count == key_capacity)
            return status_no_space;
        assert(front_page_no && back_page_no);
        assert(key_count < key_capacity);
        //在keys和pages_no中找到插入位置
        //如果当前页面未保存任何数据则直接插入
        if (key_count == 0)
        {
            keys[0] = key;
            pages_no[0] = front_page_no;
            pages_no[1] = back_page_no;
            key_count = 1;
            return status_ok;
        }
        //在keys中找到插入位置
        auto index = 0;
        for (index = 0; index < key_count; index++)
        {
            if (key <= keys[index])
                break;
        }
        //由于分裂时申请新页会作为back_page,故内部节点必定保存有front_page的页号
        //不应当存在重复key且两者的
        assert(keys[index] != key && front_page_no == pages_no[index]);
        //移动
        for (auto i = key_count - 1;i >= index;i--)
        {
            keys[i + 1] = keys[i];
            pages_no[i + 2] = pages_no[i + 1];
        }
        keys[index] = key;
        pages_no[index + 1] = back_page_no;
        key_count++;
        return status_ok;
    }

    void btree_internal_page::search(uint64 key, int& out_page_no) const
    {
        assert(key_count);
        for (auto i = 0;i < key_count;i++)
        {
            if (key < keys[i])
            {
                out_page_no = pages_no[i];
                return;
            }
        }
        out_page_no = pages_no[key_count];
    }
    int btree_internal_page::remove(int page_no)
    {
        assert(page_no >= 0);
        auto index = 0;
        for (index = 0; index <= key_count; index++)
        {
            if (pages_no[index] == page_no)
            {
                //如果index == key_count则无需额外操作
                if (index < key_count)
                {
                    for (auto i = index; i < key_count; i++)
                    {
                        keys[i] = keys[i + 1];
                        pages_no[i] = pages_no[i + 1];
                    }
                }
                key_count--;
                return status_ok;
            }
        }
        return status_not_found;
    }






}