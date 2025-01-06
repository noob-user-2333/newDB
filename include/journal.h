//
// Created by user on 25-1-2.
//

#ifndef WAL_H
#define WAL_H
#include "../third-part/roaring_bitmap/roaring.hh"
#include "utility.h"
namespace iedb {
    /*
    仅在执行update子句时生成
    仅包含两大功能
        1、将修改记录追加至该文件末尾
            所有数据追加完毕后提交
        2、遍历所有追加记录
    */
    class journal {
    private:

        struct journal_meta {
            static constexpr char magic_string[16] = "iedb_journal";
            char magic[16];
            uint64 origin_file_size;
            uint64 page_count;  //日志中保存的页面数量
            journal_meta():magic(),origin_file_size(0),page_count(0) {
                memcpy(magic,magic_string,sizeof(magic_string));
            }
        };
        struct page_meta {
            int64 page_no;
            uint64 checksum;
        };
        struct journal_page {
                page_meta meta;
                uint8 buffer[page_size];
        };
        enum class journal_status {
            normal,
            write_transaction,
            read_transaction
        };
        static constexpr int max_buffer_page = (8 *  1024 * 1024) / page_size;
        static constexpr int min_journal_size = sizeof(journal_meta) + max_buffer_page * sizeof(journal_page);
        static int page_buffer_count;
        static journal_page page_buffer[max_buffer_page];
        journal_meta meta;
        journal_status status;
        int64 read_page_count;
        int fd;
        roaring::Roaring bitmap;
        explicit journal(int fd);
        static int page_buffer_commit(int fd,int64 offset);
        static void add_page_to_buffer(int64 page_no,const void *buffer);
        int commit_buffer();
    public:
        ~journal();
        static std::unique_ptr<journal> open(const std::string& name);

        //用于遍历数据
        int begin_read_transaction();
        [[nodiscard]] uint64 get_origin_file_size() const;
        [[nodiscard]] uint64 get_page_count() const;
        int reset();
        int get_page(void * buffer,int64 &out_page_no);

        //用于追加数据
        int begin_write_transaction(uint64 origin_file_size);
        int append(int64 page_no,void * buffer);
        int commit_transaction();
        //用于清空日志
        int clear();

    };


}

#endif //WAL_H
