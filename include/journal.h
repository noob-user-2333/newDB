//
// Created by user on 25-1-2.
//

#ifndef WAL_H
#define WAL_H
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
        };
        struct page_meta {
            int64 page_no;
            uint64 checksum;
        };
        enum class journal_status {
            normal,
            write_transaction,
            read_transaction
        };

        journal_meta meta;
        journal_status status;
        int64 read_page_count;
        int fd;
        explicit journal(int fd);
    public:
        ~journal();
        static std::unique_ptr<journal> open(const std::string& name);
        static std::unique_ptr<journal> create(const std::string& name);

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

    };


}

#endif //WAL_H
