//
// Created by user on 25-2-16.
//
#include "btree_page.h"
#include "os.h"

namespace iedb
{
    int btree_page::compute_max_free_space() const
    {
        assert(data_zone_offset >= free_zone_offset);
        return page_size - static_cast<int>(sizeof(btree_page)) - static_cast<int>(sizeof(payload_meta)) *
            payload_count - payload_size_count;
    }

    int btree_page::compute_free_space() const
    {
        assert(data_zone_offset >= free_zone_offset);
        return data_zone_offset - free_zone_offset;
    }

    uint64 btree_page::compute_checksum() const
    {
        return os::calculate_checksum(this, sizeof(btree_page) - sizeof(uint64));
    }

    int btree_page::compute_free_space_without_offset(int payload_count)
    {
        return page_size - static_cast<int>(sizeof(btree_page)) - static_cast<int>(payload_count * sizeof(int));
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
        auto payload = payloads[index];
        auto offset = payload.offset;
        auto size = payload.size;
        auto payload_start = start + offset;
        slice.set(payload_start, size);
    }


    btree_page* btree_page::init(void* data, btree_page_type type,int prev_page, int next_page)
    {
        auto result = static_cast<btree_page*>(data);
        result->type = type;
        result->data_zone_offset = page_size;
        result->free_zone_offset = sizeof(btree_page);
        result->next_page = next_page;
        result->prev_page = prev_page;
        result->payload_count = 0;
        result->free_fragment_count = 0;
        result->free_fragment_offset = 0;
        result->payload_size_count = 0;
        // result->checksum = os::calculate_checksum(data, sizeof(btree_page) - sizeof(uint64));
        return result;
    }

    btree_page* btree_page::open(void* data)
    {
        auto result = static_cast<btree_page*>(data);
        // if (os::calculate_checksum(data, sizeof(btree_page) - sizeof(uint64)) != result->checksum)
        // {
            // fprintf(stderr, "当前btree页面校验和有误\n");
            // return nullptr;
        // }
        return result;
    }

    btree_page_type btree_page::get_page_type(const void* page_data)
    {
        return *static_cast<const btree_page_type*>(page_data);
    }

    int btree_page::allocate_stack_space(int size, int& out_offset)
    {
        auto free_space = compute_free_space() - static_cast<int>(sizeof(payload_meta));
        if (free_space < size)
            return status_no_space;
        //以堆栈形式弹出顶部空闲空间
        auto offset = data_zone_offset - size;
        data_zone_offset -= size;
        out_offset = offset;
        return status_ok;
    }

    int btree_page::allocate_fragment_space(int size, int& out_offset)
    {
        assert(size >= 0 && (size & 7) == 0);
        auto page_start = static_cast<uint8*>(static_cast<void*>(this));
        auto free_space = compute_free_space();
        if (free_fragment_count == 0)
            return status_not_found;
        if (free_space < static_cast<int>(sizeof(payload_meta)))
            return status_no_space;
        auto next_fragment_offset = free_fragment_offset;
        fragment_header* last_fragment = nullptr;
        //开始遍历空闲碎片链表
        while (next_fragment_offset)
        {
            auto current_fragment_offset = next_fragment_offset;
            auto fragment_start = static_cast<void*>(page_start + current_fragment_offset);
            //获取当前碎片信息
            auto fragment = static_cast<fragment_header*>(fragment_start);
            //找到满足条件的碎片
            if (fragment->size == size)
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
                //输出当前碎片偏移量
                out_offset = current_fragment_offset;
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

    int btree_page::allocate_space(int size, int& out_offset)
    {
        assert(size > 0 && size < page_size / 4);
        //由于释放时需至少8byte存放信息，故每次分配至少需要8byte
        //由于payloads仅能保存实际数据大小，无法保存实际空间大小
        //为了能正常释放空间，分配空间需向上对齐8byte
        auto actual_size = (size + 7) & (~(0x7));
        //优先从碎片中分配空间
        int offset;
        auto status = allocate_fragment_space(actual_size, offset);
        if (status != status_ok)
        {
            status = allocate_stack_space(actual_size, offset);
            //如果仍无法分配空间，则对页面空间进行整理
            if (status != status_ok)
            {
                vacuum();
                //如果仍不行说明该页确实无法存储新的payload,返回错误码
                status = allocate_stack_space(actual_size, offset);
                if (status != status_ok)
                    return status;
            }
        }
        out_offset = offset;
        //修改页表头
        payload_size_count += actual_size;
        return status_ok;
    }


    int btree_page::insert_payload_meta(uint64 key, int offset, int size)
    {
        assert(offset > sizeof(btree_page) && size > 0);
        if (free_zone_offset + sizeof(payload_meta) > data_zone_offset)
            return status_no_space;
        //查找应该插入的位置
        int index;
        for (index = 0; index < payload_count; index++)
        {
            //由于键值从小到大排列，故插入位置为第一个payload_meta大于key的区域
            if (key < payloads[index].key)
                break;
        }
        //先将index后的meta均后移1个单位
        for (auto i = payload_count - 1; i >= index; i--)
            payloads[i + 1] = payloads[i];
        //插入新meta
        payloads[index].set(key, size, offset);
        //更新页头
        payload_count++;
        free_zone_offset += sizeof(payload_meta);
        return status_ok;
    }

    void btree_page::delete_payload_meta(int index)
    {
        assert(index >= 0 && index < payload_count);
        for (auto i = index; i < payload_count; i++)
            payloads[i] = payloads[i + 1];
        payload_count--;
        free_zone_offset -= sizeof(payload_meta);
    }

    void btree_page::update_payload_meta(int index, int offset, int size)
    {
        assert(index >= 0 && index < payload_count);
        auto& payload = payloads[index];
        payload.offset = offset;
        payload.size = size;
    }

    void btree_page::free_space(int offset, int size)
    {
        auto space_size = (size + 7) & (~(0x7));
        auto start = static_cast<uint8*>(static_cast<void*>(this));
        auto space_start = static_cast<void*>(start + offset);
        //相当于将原有空间和多分配的空间一起释放
        payload_size_count -= space_size;
        //首先查看是否为堆栈顶部
        if (offset == data_zone_offset)
        {
            //仅需调整堆栈顶部即可
            data_zone_offset += space_size;
        }
        else
        {
            //否则需要将其挂载到空闲碎片空间的链表中
            auto fragment = static_cast<fragment_header*>(space_start);
            fragment->next_fragment_offset = free_fragment_offset;
            fragment->size = space_size;
            free_fragment_offset = offset;
            free_fragment_count += space_size;
        }

    }

    static int priority_compare(const void* priority1, const void* priority2)
    {
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
        payload_size_count = 0;
        //优先级由两个32bit数字组成，高32位为偏移量，低32位为index
        //首先生成优先级数组
        for (auto i = 0; i < payload_count; i++)
            priorities[i] = (static_cast<uint64>(payloads[i].offset) << 32) | i;
        //排序
        std::qsort(priorities, payload_count, sizeof(uint64), priority_compare);
        //遍历priority数组，逐个提取数据并将其放置到末尾
        auto new_data_zone_offset = page_size;
        for (auto priority : priorities)
        {
            //提取该数据对应的索引和偏移量
            auto index = static_cast<int>(priority & 0xFFFFFFFF);
            auto offset = static_cast<int>(priority >> 32);
            //提取该数据起始地址和大小
            auto data_start = page_start + offset;
            auto data_size = (payloads[index].size + 7) & (~(7));
            //将数据从后向前遍历，将其移动到页面末尾
            const auto new_data_start = page_start + new_data_zone_offset - data_size;
            os::memory_safe_copy(data_start, new_data_start, data_size);
            auto new_offset = static_cast<int>(new_data_start - page_start);
            payloads[index].offset = new_offset;
            payload_size_count += data_size;
            new_data_zone_offset -= data_size;
        }
        //初始化页头
        data_zone_offset = new_data_zone_offset;
        free_fragment_count = 0;
        free_fragment_offset = 0;
        // checksum = os::calculate_checksum(this, sizeof(btree_page) - sizeof(uint64));
    }

    int btree_page::merge(btree_page* front, btree_page* back)
    {
        assert(front && back && front->type == back->type);
        auto payload_count = front->payload_count + back->payload_count;
        auto payload_size_count = front->payload_size_count + back->payload_size_count;
        //确定是否由足够空间用于合并
        auto size_count = sizeof(btree_page) + sizeof(payload_meta) * payload_count + payload_size_count;
        if (size_count >= page_size)
            return status_no_space;
        //开始将back内容合并到front中
        //首先对front的空间进行整理
        front->vacuum();
        //获取back和front的cursor
        auto front_cursor = front->get_cursor();
        auto back_cursor = back->get_cursor();
        uint64 key;
        memory_slice slice{};
        //遍历back中的数据并将其插入front
        while (back_cursor.next() == status_ok)
        {
            back_cursor.get_payload(key, slice);
            front_cursor.insert_payload(key, slice);
        }
        //将front指向的next_page修改为back指向的next_page
        front->next_page = back->next_page;
        //更新front的checksum
        // front->checksum = os::calculate_checksum(front, sizeof(btree_page) - sizeof(uint64));
        return status_ok;
    }

    void btree_page::balance(btree_page* first, btree_page* last, uint64& out_middle_key)
    {
        assert(first && last);
        //首先对二者页面空间进行整理
        first->vacuum();
        last->vacuum();
        auto target_payload_size_count = (first->payload_size_count + last->payload_size_count) / 2;
        auto last_cursor = last->get_cursor();
        auto first_cursor = first->get_cursor();
        uint64 key;
        memory_slice slice{};
        //确定payload从哪个页面移动向哪一个页面
        if (first->payload_size_count < last->payload_count)
        {
            //从last第一个payload开始，不断移动数据到first
            last_cursor.first();
            while (first->payload_size_count < target_payload_size_count)
            {
                last_cursor.get_payload(key,slice);
                first_cursor.insert_payload(key,slice);
                last_cursor.delete_payload();
            }
        }
        else
        {
            //将从first的最后一个元素开始，不断移动数据到last
            first_cursor.last();
            while (last->payload_size_count < target_payload_size_count)
            {
                first_cursor.get_payload(key, slice);
                last_cursor.insert_payload(key, slice);
                first_cursor.delete_payload();
            }
        }
        //整理页面空间
        first->vacuum();
        last->vacuum();
        // 找到新的中间key值
        last_cursor.first();
        last_cursor.get_payload(key,slice);
        out_middle_key = key;
    }

    btree_page::btree_cursor btree_page::get_cursor()
    {
        return {this, -1};
    }
}
