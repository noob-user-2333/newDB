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
        leaf,
        free,
        overflow
    };




    /*
     *  用于表示组成btree的磁盘页
     *  仅保证payload_offset按key保存，故每条payload顺序与offset顺序不完全一致
     *  当插入新记录时优先使用空闲碎片空间
     *  若无碎片空间则以栈形式，将顶部空闲空间弹出
     *  当删除记录时，该空间将被自动挂载到由first_free_fragment组成的链表头部
     *  需要注意，每次释放与获取空间均8byte对其
     *  空闲碎片的前8byte由2个4byte的下一个碎片偏移量和当前碎片大小组成
     */
    class btree_leaf_page
    {
    public:
        /*
         * btree页面所有键值相关操作由cursor处理
         */
        class btree_cursor
        {
        private:
            btree_leaf_page* page;
            int index;

        public:
            btree_cursor(btree_leaf_page* page, int index) : page(page),index(index)
            {}
            [[nodiscard]] inline int get_index() const {return index;};
            //查找key大于等于键值的第一个节点并将当前index设置为该节点
            //如果找不到则返回status_not_found
            int search_payload_last_ge(uint64 key);
            //查找key小于等于键值的最后一个节点并将当前index设置为该节点
            //如果找不到则返回status_not_found
            int search_payload_first_le(uint64 key);
            int insert_payload(uint64 key, const memory_slice & data);
            int next();
            int previous();
            int last();
            int first();
            void get_payload(uint64&out_key,memory_slice&out_data) const;
            int update_payload(const memory_slice & new_data);
            //若删除后payload_size_count < page_size /4，则认为需要进行调整，并输出标志
            void delete_payload(bool & out_need_adjust);
        };
        struct payload_meta
        {
            int offset;
            int size;
            uint64 key;
            void set(uint64 key,int size,int offset)
            {
                this->key = key;
                this->size = size;
                this->offset = offset;
            }
        };
        struct fragment_header
        {
            int next_fragment_offset;
            int size;
        };
    private:
        /*
         * btree页面空间相关操作由私有成员函数处理
         */


        [[nodiscard]] int compute_max_free_space() const;
        [[nodiscard]] int compute_free_space() const;
        [[nodiscard]] uint64 compute_checksum()const;
        [[nodiscard]] int compute_slice_offset_for_page(const memory_slice & slice) const;
        static int compute_free_space_without_offset(int payload_count);
        //allocate函数仅从对应位置获取空间但不会修改payloads
        int allocate_stack_space(int size,int & out_offset);
        int allocate_fragment_space(int size,int & out_offset);
        //每次至少分配8字节,8字节对齐，且保证栈剩余空间大于一个payload_meta
        int allocate_space(int size,int& out_offset);
        int insert_payload_meta(uint64 key,int offset,int size);
        void update_payload_meta(int index,int offset,int size);
        void delete_payload_meta(int index);
        //释放slice对应的页面空间
        void free_space(int offset,int size);
        //回收空闲空间并进行整理
        void vacuum();
        void get_payload(int index,memory_slice&slice);
    public:
        btree_page_type type;
        int data_zone_offset;
        int free_zone_offset;
        int next_page;
        int prev_page;
        int payload_count;
        int free_fragment_count;
        int free_fragment_offset;
        int payload_size_count;
        int reserved;
        uint64 checksum;
        payload_meta payloads[];
        btree_leaf_page() = delete;


        static btree_leaf_page* init(void* data, int prev_page, int next_page);
        //如果checksum和计算出的不同则返回空指针
        static btree_leaf_page* open(void* data);
        //将两者的payload进行平衡，out_middle_key为last页面的第一个key值
        static void balance(btree_leaf_page* first, btree_leaf_page* last,uint64 & out_middle_key);
        //将back指向的页面的内容合并到front中
        //注:若二者不足以合并到同一页面返回status_no_space
        static int merge(btree_leaf_page* front, btree_leaf_page* back);
        static btree_page_type get_page_type(const void* page_data);
        void get_payload(int index,uint64 & out_key,memory_slice & data);
        btree_cursor get_cursor();



    };
}


#endif //BTREE_PAGE_H
