//
// Created by user on 25-2-17.
//
#include "btree_page.h"

namespace iedb
{
    uint64 btree_page::btree_cursor::extract_key_from_slice(const memory_slice& slice)
    {
        const auto key_ptr = static_cast<uint64*>(slice.buffer);
        return *key_ptr;
    }

    int btree_page::btree_cursor::search_payload(uint64 key, uint64& out_key, memory_slice& out_data)
    {
        //首先确定该页是否存在key大于查询key的payload
        memory_slice data{};
        page->get_payload(page->payload_count - 1, data);
        auto max_key = extract_key_from_slice(data);
        if (max_key < key)
            return status_not_found;
        //遍历所有payload，找出最接近所需key的
        for (auto i = 0;i < page->payload_count;i++)
        {
            page->get_payload(i,data);
            const auto current_key = extract_key_from_slice(data);
            if (current_key >= key)
            {
                out_key = current_key;
                out_data = data;
                index = i;
                slice = data;
                return status_ok;
            }
        }
        throw std::runtime_error("should not run here");
    }






}
