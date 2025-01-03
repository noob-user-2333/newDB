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
        transaction_read,
        transaction_write
    };

    //每个文件默认存在一个对应的wal日志文件
    struct wal_header {
        static constexpr char magic_string[16] = "iedb_wal";
        char magic[16];
        uint64 wal_file_size;
        uint32 record_count;
        wal_header():magic(),wal_file_size(0),record_count(0) {
            memcpy(magic,magic_string, sizeof(magic_string));
        }
        [[nodiscard]] int check_header() const;
    };
    struct file_header {
        static constexpr char magic_string[16] = "iedb db file";
        char magic[16];
        uint64 file_size;
        uint64 record_count;
        file_header():magic(), file_size(0), record_count(0) {
            memcpy(magic, magic_string, sizeof(magic_string));
        }

    };

    class db_file_manager {
    private:
        //wal私有数据
        wal_header _wal_header;  //用于暂存日志头
        std::string wal_name;
        int fd_wal;
        //数据库文件私有数据
        std::string filename;
        int fd;
        file_status status;
        file_header _file_header;
        db_file_manager(std::string filename,int fd);
        int wal_create();
        int wal_delete();
        int wal_commit();
        int wal_recovery();
        int wal_add_record(int64 offset_in_origin,void * record,uint64 record_size);
    public:
        ~db_file_manager();
        static std::unique_ptr<db_file_manager> open(const char * name,bool create = true);
        int begin_write_transaction();
        int begin_read_transaction();
        int commit_transaction();
        int append(void * buffer,uint64 size);
        int read(int64 offset,void *buffer,uint64 size);
        int write(int64 offset,void *buffer,uint64 size);

    };
}



#endif //DB_FILE_MANAGER_H
