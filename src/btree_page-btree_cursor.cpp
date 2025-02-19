//
// Created by user on 25-2-17.
//

#include "btree_page.h"

namespace iedb
{
    int btree_page::btree_cursor::search_payload(uint64 key)
    {
        assert(page->payload_count >= 0);

        for (auto current_index = 0;current_index < page->payload_count;current_index++)
        {
            const auto& payload = page->payloads[current_index];
            const auto current_key = payload.key;
            if (current_key >= key)
            {
                index = current_index;
                return status_ok;
            }
        }
        return status_not_found;
    }
    int btree_page::btree_cursor::insert_payload(uint64 key, const memory_slice& data)
    {
        const auto size = static_cast<int>(data.size);
        int offset;
        auto status = page->allocate_space(size,offset);
        if (status!= status_ok)
            return status;
        //将数据复制到指定位置
        auto page_start = reinterpret_cast<uint8*>(page);
        auto data_start = page_start + offset;
        memcpy(data_start,data.buffer,data.size);
        status = page->insert_payload_meta(key,offset,size);
        if (status!= status_ok)
        {
            page->free_space(offset, size);
            return status;
        }
        return status_ok;
    }

    int btree_page::btree_cursor::next()
    {
        if (index + 1 >= page->payload_count)
            return status_out_of_range;
        index++;
        return status_ok;
    }
    int btree_page::btree_cursor::previous()
    {
        if (index == 0  || page->payload_count < index - 1)
            return status_out_of_range;
        index--;
        return status_ok;
    }
    int btree_page::btree_cursor::first()
    {
        if (page->payload_count == 0)
            return status_out_of_range;
        index = 0;
        return status_ok;
    }
    int btree_page::btree_cursor::last()
    {
        if (page->payload_count == 0)
            return status_out_of_range;
        index = page->payload_count - 1;
        return status_ok;
    }
    int btree_page::btree_cursor::get_payload(uint64& out_key, memory_slice& out_data)
    {
        if (index < 0 || index >= page->payload_count)
            return status_out_of_range;
        const auto& payload = page->payloads[index];
        out_key = payload.key;
        out_data.set(reinterpret_cast<uint8*>(page) + payload.offset, payload.size);
        return status_ok;
    }

    int btree_page::btree_cursor::update_payload(const memory_slice& new_data)
    {
        //首先确定当前使用空间与new_data是否一致
        auto& payload = page->payloads[index];
        auto actual_payload_size = (payload.size + 7) & (~7);
        auto actual_new_data_size = (new_data.size + 7) & (~7);
        //如果一致，仅需将新数据直接拷贝到指定位置
        if (actual_payload_size == actual_new_data_size)
        {
            auto page_start = reinterpret_cast<uint8*>(page);
            auto start = page_start + payload.offset;
            payload.size = static_cast<int>(new_data.size);
            memcpy(start,new_data.buffer,new_data.size);
            return status_ok;
        }
        //首先确定当前页面是否能存放新数据
        auto max_free_space = page->compute_max_free_space();
        if (max_free_space + payload.size < new_data.size)
            return status_no_space;
        //然后释放旧空间
        page->free_space(payload.offset, payload.size);
        //然后重新分配新空间
        int offset;
        auto size = static_cast<int>(new_data.size);
        auto status = page->allocate_space(size, offset);
        if (status!= status_ok)
            return status;
        //然后更新payloads
        payload.offset = offset;
        payload.size = size;
        //然后拷贝new_data
        memcpy(reinterpret_cast<uint8*>(page) + offset, new_data.buffer, new_data.size);
        return status_ok;
    }


    int btree_page::btree_cursor::delete_payload()
    {
        //先释放空间
        const auto& payload = page->payloads[index];
        page->free_space(payload.offset, payload.size);
        //然后从payloads中删除
        page->delete_payload_meta(index);
        if (index >= page->payload_count)
            index--;
        return status_ok;
    }




}
