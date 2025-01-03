//
// Created by user on 25-1-2.
//

#ifndef FILE_H
#define FILE_H

#include "utility.h"
#include "journal.h"
namespace iedb {
    class file{
        private:
            struct file_page {
                int64 page_no;
                uint8 buffer[page_size];
            };
            int fd;
            std::unique_ptr<journal> j;


            file(int fd,std::unique_ptr<journal> & j);
        public:
            ~file();
            static std::unique_ptr<file> open(const std::string& name);
            int begin_read_transaction();


    };




}

#endif //FILE_H
