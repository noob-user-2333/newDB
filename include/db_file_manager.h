//
// Created by user on 24-12-20.
//

#ifndef DB_FILE_MANAGER_H
#define DB_FILE_MANAGER_H

#include "os.h"
#include "../third-part/roaring_bitmap/roaring.hh"
namespace iedb {
    enum class file_status {
        error,
        normal,
        transaction
    };
    /*
     *  对于db_file而言，其第0号页被隐藏，及所有写入实际从第1页开始，
     *  实际写入偏移量 = 请求写入 + page_size
     *  在首次进行写入时默认会对0号页进行备份
     *
     */
    struct journal_header {
        static constexpr char magic_string[16] = "iedb_journal";
        char magic[16];
        uint64 origin_file_size;
        uint32 page_count;
        journal_header():magic(),origin_file_size(0),page_count(0) {
            memcpy(magic,magic_string, sizeof(magic_string));
        }
        [[nodiscard]] int check_header() const;
        static int64 get_journal_page_offset(uint32 page_no);
    };

    class db_file_manager {
    private:
        int fd_journal;
        roaring::Roaring bitmap;
        std::string filename;
        std::string journal_name;
        int fd;
        file_status status;
        uint8 meta_page[page_size];//用于暂存元数据
        journal_header header;  //用于暂存日志头
        db_file_manager(std::string filename,int fd);
        static int check_sum_compute(void * page_data);
        int create_journal();
        int delete_journal();
        int backup_page(uint32 page_no,void *buffer);
    public:
        ~db_file_manager();
        static std::unique_ptr<db_file_manager> open(const char * name,bool create = true);
        int begin_transaction();
        int commit_transaction();
        int rollback_transaction();
        int append(void * buffer,uint64 size);
        int read(void *buffer,uint64 size);
        int write(void *buffer,uint64 size);

    };
}



#endif //DB_FILE_MANAGER_H
