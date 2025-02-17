//
// Created by user on 24-12-5.
//

#ifndef OS_H
#define OS_H
#include "utility.h"

namespace iedb {
    class os {
    public:
        struct io_vec {
            void *io_base;
            uint64 iov_len;
            void set(void *base, const uint64 len) {
                io_base = base;
                iov_len = len;
            };
        };
        static const int open_mode_read;
        static const int open_mode_write;
        static const int open_mode_append;
        static const int open_mode_read_write;
        static const int open_mode_create;
        static const int open_mode_truncate;
        static const int open_mode_excl;
        static const int access_mode_file_exists;
        static const int access_mode_can_read;
        static const int access_mode_can_write;
        static const int access_mode_can_execute;
        static const int seek_mode_set;
        static const int seek_mode_current;
        static const int seek_mode_end;
        static int open(const char *path,int mode,int flags,int& out_fd);
        static int access(const char *path,int mode);
        static int close(int fd);
        static int seek(int fd,int64 offset,int mode,int64 & out_current_offset);
        static int write(int fd,int64 offset,const void *buf,uint64 count);
        static int write(int fd,const void *buf,uint64 count);
        // static int writev(int fd,const io_vec *iov, int iov_count);
        static int read(int fd,int64 offset,void *buf,uint64 count);
        static int read(int fd,void *buf,uint64 count);
        // static int readv(int fd,const io_vec *iov, int iov_count);
        static int mkdir(const char *path);
        static int fallocate(int fd,int64 offset,int64 length);
        static int fdatasync(int fd);
        static int unlink(const char *path);
        static int get_file_size(int fd,int64 & out_size);
        static int ftruncate(int fd, int64 length);
        static int is_directory(const char *path);
        static uint64 calculate_checksum(const void * buffer, uint64 size);
        static uint64 get_real_time_clock();
        static void memory_safe_copy(void *src,void* dest,int size);
    };
}


#endif //OS_H
