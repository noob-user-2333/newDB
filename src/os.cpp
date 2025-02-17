//
// Created by user on 24-12-5.
//

#include "os.h"

#include <immintrin.h>
#include <avx2intrin.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>

namespace iedb
{
    const int os::open_mode_read = O_RDONLY;
    const int os::open_mode_write = O_WRONLY;
    const int os::open_mode_append = O_APPEND;
    const int os::open_mode_read_write = O_RDWR;
    const int os::open_mode_create = O_CREAT;
    const int os::open_mode_truncate = O_TRUNC;
    const int os::open_mode_excl = O_EXCL;
    const int os::access_mode_file_exists = F_OK;
    const int os::access_mode_can_read = R_OK;
    const int os::access_mode_can_write = W_OK;
    const int os::access_mode_can_execute = X_OK;
    const int os::seek_mode_set = SEEK_SET;
    const int os::seek_mode_current = SEEK_CUR;
    const int os::seek_mode_end = SEEK_END;

    static int errno_to_status_code(const int error_number)
    {
        switch (error_number)
        {
        case 0:
            return status_ok;
        case ENOSPC:
            return status_no_space;
        case EACCES:
            return status_not_access;
        case EIO:
            return status_io_error;
        case EINVAL:
            return status_invalid_argument;
        case EBADF:
            return status_invalid_fd;
        case EOVERFLOW:
            return status_argument_overflow;
        case EEXIST:
            return status_file_exists;
        default:
            return status_error;
        }
    }


    int os::open(const char* path, int mode, int flags, int& out_fd)
    {
        const int fd = ::open(path, mode, flags);
        if (fd < 0)
            return errno_to_status_code(errno);
        out_fd = fd;
        return status_ok;
    }

    int os::access(const char* path, int mode)
    {
        if (::access(path, mode))
            return status_error;
        return status_ok;
    }

    int os::seek(int fd, int64 offset, int mode, int64& out_current_offset)
    {
        auto _offset = lseek(fd, offset, mode);
        if (_offset == -1)
            return errno_to_status_code(errno);
        out_current_offset = _offset;
        return status_ok;
    }


    int os::close(int fd)
    {
        if (::close(fd) < 0)
            return errno_to_status_code(errno);
        return status_ok;
    }

    int os::write(int fd, int64 offset, const void* buf, uint64 count)
    {
        auto result = lseek(fd, offset, SEEK_SET);
        int error;
        if (result == -1)
            return errno_to_status_code(errno);
        do
        {
            result = ::write(fd, buf, count);
            error = errno;
        }
        while (result == -1 && error == EINTR);
        if (result == -1)
            return errno_to_status_code(error);
        return status_ok;
    }

    int os::write(int fd, const void* buf, uint64 count)
    {
        auto result = 0L;
        auto error = 0;
        do
        {
            result = ::write(fd, buf, count);
            error = errno;
        }
        while (result == -1 && error == EINTR);
        if (result == -1)
            return errno_to_status_code(error);
        return status_ok;
    }

    // int os::writev(int fd, const io_vec* io_vec, int count) {
    //     int error;
    //     int64 result;
    //     do {
    //         result = ::writev(fd, static_cast<iovec*>((void*)io_vec), count);
    //         error = errno;
    //     }while (result == -1 && error == EINTR);
    //     if (result == -1)
    //         return errno_to_status_code(error);
    //     return status_ok;
    // }
    int os::read(int fd, int64 offset, void* buf, uint64 count)
    {
        auto result = lseek(fd, offset, SEEK_SET);
        if (result == -1)
            return errno_to_status_code(errno);
        //在未读取到count字节前且未发生故障前不断尝试读取数据
        uint64 got = 0;
        do
        {
            result = ::read(fd, buf, count);
            if (result == -1)
            {
                if (errno == EINTR)
                    continue;
                return errno_to_status_code(errno);
            }
            got += result;
            if (result == 0)
            {
                if (got == 0)
                    return status_file_read_complete;
                memset(static_cast<char*>(buf) + got, 0, count - got);
                return status_io_error_short_read;
            }
        }
        while (got < count);

        return status_ok;
    }

    int os::read(int fd, void* buf, uint64 count)
    {
        auto result = 0L;
        //在未读取到count字节前且未发生故障前不断尝试读取数据
        uint64 got = 0;
        do
        {
            result = ::read(fd, buf, count);
            if (result == -1)
            {
                if (errno == EINTR)
                    continue;
                return errno_to_status_code(errno);
            }
            got += result;
            if (result == 0)
            {
                if (got == 0)
                    return status_file_read_complete;
                memset(static_cast<char*>(buf) + got, 0, count - got);
                return status_io_error_short_read;
            }
        }
        while (got < count);
        return status_ok;
    }


    // int os::readv(int fd, const io_vec* io_vec, int count) {
    //     int error;
    //     int64 result;
    //     do {
    //         result = ::readv(fd, static_cast<iovec*>((void*)io_vec), count);
    //         error = errno;
    //     }while (result == -1 && error == EINTR);
    //     if (result == -1)
    //         return errno_to_status_code(error);
    //     return status_ok;
    // }
    int os::mkdir(const char* path)
    {
        auto result = ::mkdir(path, 0777);
        if (result == -1)
            return errno_to_status_code(errno);
        return status_ok;
    }

    int os::fallocate(int fd, int64 offset, int64 length)
    {
        int error;
        do
        {
            error = posix_fallocate(fd, offset, length);
        }
        while (error == EINTR);
        return errno_to_status_code(error);
    }

    int os::fdatasync(int fd)
    {
        int status, error = 0;
        do
        {
            status = ::fdatasync(fd);
            error = errno;
        }
        while (status < 0 && error == EINTR);
        if (status < 0)
            return errno_to_status_code(error);
        return status_ok;
    }

    int os::unlink(const char* path)
    {
        auto status = ::unlink(path);
        if (status < 0)
            return errno_to_status_code(errno);
        return status_ok;
    }

    int os::get_file_size(int fd, int64& out_size)
    {
        assert(fd > 0);
        struct stat st{};
        if (::fstat(fd, &st) < 0)
            return errno_to_status_code(errno);
        out_size = st.st_size;
        return status_ok;
    }

    // int os::mmap(int fd, int64 offset, uint64 length, void *&out_start) {
    //     out_start = ::mmap(nullptr,length,PROT_READ | PROT_WRITE,MAP_SHARED,fd,offset);
    //     if (out_start == ((void*)(-1)))
    //         return errno_to_status_code(errno);
    //     return status_ok;
    // }
    // int os::munmap(void *start, uint64 length) {
    //     int status = ::munmap(start,length);
    //     if (status != status_ok)
    //         return errno_to_status_code(errno);
    //     return status_ok;
    // }

    int os::ftruncate(int fd, int64 length)
    {
        int status, error = 0;
        do
        {
            status = ::ftruncate(fd, length);
            error = errno;
        }
        while (status != status_ok && error == EINTR);
        if (status != status_ok)
            return errno_to_status_code(error);
        return status_ok;
    }

    int os::is_directory(const char* path)
    {
        struct stat info{};
        if (::stat(path, &info) != 0)
        {
            // 文件不存在
            return status_file_not_exists;
        }
        // 判断是否为目录
        if ((info.st_mode & S_IFDIR) != 0)
            return status_ok;
        return status_error_file_type;
    }

    uint64_t os::calculate_checksum(const void* buffer, uint64 size)
    {
        const auto data = static_cast<const char*>(buffer);
        __m256i checksum = _mm256_setzero_si256(); // 初始化 256 位寄存器为 0
        size_t i = 0;

        // 每次处理 32 字节（4 个 64 位整数）
        for (; i + 32 <= size; i += 32)
        {
            const __m256i chunk = _mm256_loadu_si256((__m256i*)(data + i)); // 加载 32 字节数据
            checksum = _mm256_add_epi64(checksum, chunk); // 逐 64 位并行累加
        }

        // 汇总 256 位寄存器的结果到标量
        uint64_t temp[4];
        _mm256_storeu_si256((__m256i*)temp, checksum); // 存储到临时数组
        uint64_t total_sum = temp[0] + temp[1] + temp[2] + temp[3]; // 求和

        // 处理剩余不足 32 字节的部分（逐 8 字节处理）
        for (; i + 8 <= size; i += 8)
        {
            uint64_t chunk = *(uint64_t*)(data + i); // 加载 8 字节数据
            total_sum += chunk;
        }

        // 处理不足 8 字节的尾部数据（逐字节处理）
        for (; i < size; ++i)
        {
            total_sum += data[i];
        }

        return total_sum;
    }

    uint64 os::get_real_time_clock()
    {
        timespec ts{};
        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_nsec;
    }

    void os::memory_safe_copy(void* src, void* dest, int size)
    {
        auto offset = reinterpret_cast<int64>(dest) - reinterpret_cast<int64>(src);
        if (offset < 0 || offset >= size)
        {
            ::memcpy(dest,src,size);
            return;
        }
        //对于dest在src后方，且二者区域存在重叠的情况需要特殊处理
        auto src_start = static_cast<uint8*>(src);
        auto src_end = src_start + size - 1;

        // 先处理无法对齐 32 字节的部分
        size_t remainder = size % 32;
        for (size_t i = 0; i < remainder; ++i) {
            src_end[offset] = src_end[0];
            src_end--;
        }
        // 以 256-bit（32 字节）为单位进行拷贝
        auto src_end_256 = reinterpret_cast<__m256i*>(src_end - 31);
        auto dest_end_256 = reinterpret_cast<__m256i*>(src_end - 31 + offset);

        for (size_t i = 0; i < size / 32; ++i) {
            __m256i data = _mm256_loadu_si256(src_end_256);
            _mm256_storeu_si256(dest_end_256, data);
            src_end_256--;
            dest_end_256--;
        }
    }

}
