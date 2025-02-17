//
// Created by user on 25-2-16.
//

#ifndef BTREE_PAGE_H
#define BTREE_PAGE_H
#include"utility.h"
/*
 * 用于对B树页面进行处理
 */

namespace iedb
{
    enum class btree_page_type
    {
        unknown,
        internal,
        leaf
    };

    /*
     * 用于表示一块内存
     */
    struct memory_slice
    {
        int size;
        void* buffer;
        inline void set( void* buffer,int size)
        {
            this->size = size;
            this->buffer = buffer;
        }
    };


    /*
     *  用于表示组成btree的磁盘页
     *  仅保证payload_offset按key保存，每条payload顺序与offset顺序不完全一致
     *  当插入新记录时优先使用空闲碎片空间
     *  若无碎片空间则以栈形式，将顶部空闲空间弹出
     *  当删除记录时，该空间将被自动挂载到由first_free_fragment组成的链表头部
     *  需要注意，每次释放与获取空间均8byte对其
     *  空闲碎片的前8byte由2个4byte的下一个碎片偏移量和当前碎片大小组成
     */
    class btree_page
    {
    public:
        class btree_cursor
        {
        private:
            btree_page* page;
            memory_slice slice;
            int index;
            static uint64 extract_key_from_slice(const memory_slice & slice);

        public:
            btree_cursor(btree_page* page, int index) : page(page), slice(),index(index)
            {}
            //查找键值大于等于key的第一个树节点
            int search_payload(uint64 key,uint64&out_key,memory_slice&out_data);
            int next();
            int previous();
            int get_payload(uint64&out_key,memory_slice&out_data);
            int update_payload(const memory_slice & new_data);
            int delete_payload();
        };
    private:
        struct fragment_header
        {
            int next_fragment_offset;
            int size;
        };
        [[nodiscard]] int compute_free_space() const;
        [[nodiscard]] uint64 compute_checksum()const;
        [[nodiscard]] int compute_slice_offset_for_page(const memory_slice & slice) const;
        static int compute_free_space_without_offset(int payload_count);
        int allocate_stack_space(int size,memory_slice& new_slice);
        int allocate_fragment_space(int size,memory_slice& new_slice);
        void insert_to_offset_array(int index,int offset,int offset_array_size);
        void delete_from_offset_array(int index,int offset_array_size);
        //释放slice对应的页面空间
        void free_space(int index);
        //回收空闲空间并进行整理
        void vacuum();
        void get_payload(int index,memory_slice&slice);
    public:
        btree_page_type type;
        int data_zone_offset;
        int free_zone_offset;
        int next_page;
        int payload_count;
        int free_fragment_count;
        int free_fragment_offset;
        int payload_size_count;
        uint64 checksum;
        int payload_offset[];
        btree_page() = delete;


        static btree_page* init(void* data, btree_page_type type, int next_page);
        //如果checksum和计算出的不同则返回空指针
        static btree_page* open(void* data);
        //将两者
        static int balance(btree_page* first, btree_page* last);
        //将back指向的页面的内容合并到front中
        //注:若二者不足以合并到同一页面返回status_no_space
        static int merge(btree_page* front, btree_page* back);

        btree_cursor get_cursor();



    };
}


#endif //BTREE_PAGE_H
