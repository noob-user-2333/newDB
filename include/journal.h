//
// Created by user on 25-1-2.
//

#ifndef WAL_H
#define WAL_H
#include "../third-part/roaring_bitmap/roaring.hh"
#include "utility.h"
namespace iedb {
    /*
        日志文件，备份将被修改页面的原页面
        备份规则:
            1 将原页面复制入对应缓存
            2 当缓存满或提交日志时将内容写入日志文件并修改日志头
        核心:
            将指定文件作为日志文件并提供操作接口
    */
    class journal {
    private:
        struct journal_meta {
            static constexpr char magic_string[16] = "iedb_journal";
            char magic[16];
            int64 origin_file_size;
            uint64 page_count;  //日志中保存的页面数量
            journal_meta():magic(),origin_file_size(0),page_count(0) {
                memcpy(magic,magic_string,sizeof(magic_string));
            }
        };
        struct journal_page {
            int page_no;
            uint64 checksum;
            uint8 buffer[page_size];
        };
        enum class journal_status
        {
            normal,
            read,
            write
        };

        static constexpr int max_buffer_page = (8 *  1024 * 1024) / page_size;

        int page_buffer_count{};
        journal_page page_buffer[max_buffer_page]{};
        journal_meta meta;
        journal_status status;
        int fd{};
        roaring::Roaring bitmap;
        void add_page_to_buffer(int page_no,const void *buffer);
        explicit journal(int fd);
    public:
        ~journal();
        static std::unique_ptr<journal> open(const std::string & path);

        int commit_buffer();
        //读取日志页面，以对数据库进行恢复
        //偶发情况，无需考虑性能
        int begin_read_transaction();
        [[nodiscard]] int64 get_origin_file_size() const;
        [[nodiscard]] uint64 get_page_count() const;
        int read_next_page(void * buffer,int &out_page_no) const;

        //用于添加原页面
        int begin_write_transaction(int64 origin_file_size);
        int add_page(int page_no, const void * buffer);
        int commit_transaction();
        int clear();

        int close();


    };


}

#endif //WAL_H
