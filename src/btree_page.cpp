//
// Created by user on 25-2-16.
//
#include "btree_page.h"
#include "os.h"

namespace iedb
{
    int btree_page::compute_free_space() const
    {
        return data_zone_offset - free_zone_offset;
    }
    uint64 btree_page::compute_checksum() const
    {
        return os::calculate_checksum(this,sizeof(btree_page) - sizeof(uint64));
    }
    int btree_page::compute_free_space_without_offset(int payload_count)
    {
        return static_cast<int>(static_cast<uint64>(page_size) - sizeof(btree_page) - payload_count * sizeof(int));
    }

    int btree_page::compute_slice_offset_for_page(const memory_slice& slice) const
    {
        auto start = static_cast<const void*>(this);
        const void* slice_data = slice.buffer;
        const auto offset = reinterpret_cast<int64>(slice_data) - reinterpret_cast<int64>(start);
        assert(offset > 0);
        return static_cast<int>(offset);
    }

    void btree_page::get_payload(int index, memory_slice& slice)
    {
        assert(index >= 0 && index < payload_count);
        auto start = static_cast<uint8*>(static_cast<void*>(this));
        auto offset = payload_offset[index];
        auto payload_start = reinterpret_cast<int*>(start + offset);
        slice.set(payload_start + 1,*payload_start);
    }


    btree_page* btree_page::init(void* data, btree_page_type type, int next_page)
    {
        auto result = static_cast<btree_page*>(data);
        result->type = type;
        result->data_zone_offset = page_size;
        result->free_zone_offset = sizeof(btree_page);
        result->next_page = next_page;
        result->payload_count = 0;
        result->free_fragment_count = 0;
        result->free_fragment_offset = 0;
        result->payload_size_count = 0;
        result->checksum = os::calculate_checksum(data,sizeof(btree_page) - sizeof(uint64));
        return result;
    }
    btree_page* btree_page::open(void* data)
    {
        auto result = static_cast<btree_page*>(data);
        if (os::calculate_checksum(data, sizeof(btree_page) - sizeof(uint64))!= result->checksum)
        {
            fprintf(stderr, "当前btree页面校验和有误\n");
            return nullptr;
        }
        return result;
    }

    int btree_page::allocate_stack_space(int size,memory_slice & new_slice)
    {
        //需要额外4byte存放实际大小
        size += 4;
        auto free_space = compute_free_space() - sizeof(int);
        if (free_space < size)
            return status_no_space;
        //以堆栈形式弹出顶部空闲空间
        auto page_start = static_cast<uint8*>(static_cast<void*>(this));
        auto offset = data_zone_offset - size;
        payload_offset[payload_count] = offset;
        payload_count++;
        data_zone_offset -= size;
        free_zone_offset += sizeof(int);
        auto start = page_start + offset;
        *reinterpret_cast<int*>(start) = size;
        new_slice.set(start + sizeof(int),size);
        return status_ok;
    }
    int btree_page::allocate_fragment_space(int size, memory_slice& new_slice)
    {
        //需要额外4byte存放实际大小
        size += 4;
        auto page_start = static_cast<uint8*>(static_cast<void*>(this));
        if (free_fragment_count == 0)
            return status_no_space;
        auto next_fragment_offset = free_fragment_offset;
        fragment_header * last_fragment = nullptr;
        //开始遍历空闲碎片链表
        while (next_fragment_offset)
        {
            auto fragment_start = static_cast<void*>(page_start + next_fragment_offset);
            //获取当前碎片信息
            auto fragment = static_cast<fragment_header*>(fragment_start);
            //找到满足条件的碎片
            if (fragment->size >= size)
            {
                //将该碎片从链表中释放
                if (last_fragment)
                {
                    //将上一个碎片节点的下一节点偏移量
                    last_fragment->next_fragment_offset = fragment->next_fragment_offset;
                }
                else
                {
                    //对于链表头碎片节点大小符合请求的情况,仅需修改页头
                    free_fragment_offset = fragment->next_fragment_offset;
                }
                *static_cast<int*>(fragment_start) = size;
                //将碎片作为memory_slice传出
                new_slice.set(fragment_start + sizeof(int),size);
                //修改页头
                free_fragment_count -= size;
                return status_ok;
            }
            //查询下一个碎片
            last_fragment = fragment;
            next_fragment_offset = fragment->next_fragment_offset;
        }
        return status_no_space;
    }

    void btree_page::insert_to_offset_array(int index, int offset, int offset_array_size)
    {
        assert(index >= 0 && index < offset_array_size);
        for (auto i = offset_array_size - 1;i >= index;i--)
            payload_offset[i + 1] = payload_offset[i];
        payload_offset[index] = offset;
    }
    void btree_page::delete_from_offset_array(int index, int offset_array_size)
    {
        assert(index >= 0 && index < offset_array_size);
        for (auto i = index;i < offset_array_size;i++)
            payload_offset[i] = payload_offset[i + 1];
    }

    void btree_page::free_space(int index)
    {
        assert(index >= 0 && index < payload_count);
        auto offset = payload_offset[index];
        auto start = static_cast<uint8*>(static_cast<void*>(this));
        auto space_start = static_cast<void*>(start + offset);
        auto space_size = *static_cast<int*>(space_start);
        //首先查看是否为堆栈顶部
        if (offset == data_zone_offset)
        {
            //仅需调整堆栈顶部即可
            data_zone_offset -= space_size;
        }else
        {
            //否则需要将其挂载到空闲碎片空间的链表中
            auto fragment = static_cast<fragment_header*>(space_start);
            fragment->next_fragment_offset = free_fragment_offset;
            fragment->size = space_size;
            free_fragment_offset = offset;
        }
    }

    static int priority_compare(const void* priority1, const void* priority2) {
        const unsigned long num1 = *static_cast<const unsigned long*>(priority1);
        const unsigned long num2 = *static_cast<const unsigned long*>(priority2);

        // 返回 num2 - num1，确保大到小排序
        if (num1 < num2) return 1;
        if (num1 > num2) return -1;
        return 0;
    }
    void btree_page::vacuum()
    {
        auto page_start = reinterpret_cast<uint8*>(this);
        uint64 priorities[payload_count];
        //优先级由两个32bit数字组成，高32位为偏移量，低32位为index
        //首先生成优先级数组
        for (auto i = 0;i < payload_count;i++)
            priorities[i] = (static_cast<uint64>(payload_offset[i]) << 32) | i;
        //排序
        std::qsort(priorities,payload_count,sizeof(uint64),priority_compare);
        //遍历priority数组，逐个提取数据并将其放置到末尾
        auto new_data_zone_offset = page_size;
        for (auto priority : priorities)
        {
            //提取该数据对应的索引和偏移量
            auto index = static_cast<int>(priority & 0xFFFFFFFF);
            auto offset = static_cast<int>(priority >> 32);
            //提取该数据起始地址和大小
            auto data_start = page_start + offset;
            auto data_size = *reinterpret_cast<int*>(data_start);
            //将数据从后向前遍历，将其移动到页面末尾
            const auto new_data_start = page_start + new_data_zone_offset - data_size;
            os::memory_safe_copy(data_start,new_data_start,data_size);
            auto new_offset = static_cast<int>(new_data_start - page_start);
            payload_offset[index] = new_offset;
            new_data_zone_offset -= data_size;
        }
        //初始化页头
        data_zone_offset = new_data_zone_offset;
        free_fragment_count = 0;
        free_fragment_offset = 0;
        checksum = os::calculate_checksum(this,sizeof(btree_page) - sizeof(uint64));
    }

    // int btree_page::merge(btree_page* front, btree_page* back)
    // {
    //     assert(front && back && front->type == back->type);
    //     auto payload_count = front->payload_count + back->payload_count;
    //     auto payload_size_count = front->payload_size_count + back->payload_size_count;
    //     //确定是否由足够空间用于合并
    //     auto free_space = btree_page::compute_free_space_without_offset(payload_count);
    //     if (free_space < payload_size_count)
    //         return status_no_space;
    //     //开始将back内容合并到front中
    //     //首先对front的空间进行整理
    //     front->vacuum();
    //     //获取back和front的cursor
    //
    //
    // }







    btree_page::btree_cursor btree_page::get_cursor()
    {
        return {this,0};
    }




}