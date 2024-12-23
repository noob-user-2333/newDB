//
// Created by user on 24-12-20.
//

#ifndef DB_FILE_MANAGER_H
#define DB_FILE_MANAGER_H

#include "os.h"
#include "roaring.hh"
namespace iedb {
    enum class file_status {
        normal,
        transaction
    };




    class db_file_manager {
    private:
        std::string filename;
        int fd;
        int fd_journal;
        file_status status;
        roaring::Roaring bitmap;
        db_file_manager(std::string filename,int fd);
    public:
        ~db_file_manager();
        static std::unique_ptr<db_file_manager> open(const char * name,bool create = true);
        int insert(void * buffer,uint64 size);
        int read(void *buffer,uint64 size);
        int beginTransaction();
        int commitTransaction();
        int rollbackTransaction();
        int write(void *buffer,uint64 size);

    };
}



#endif //DB_FILE_MANAGER_H
