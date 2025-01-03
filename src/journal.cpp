//
// Created by user on 25-1-2.
//
#include "journal.h"
#include "os.h"

namespace iedb {
#define maybe_error(x){_status = x; if(_status != status_ok) goto error_handle;}
    journal::journal(const int fd):read_page_count(0),meta(),fd(fd),status(journal_status::normal) {
    }
    journal::~journal() {
        os::close(fd);
    }
    std::unique_ptr<journal> journal::open(const std::string &name) {
        journal_meta meta{};
        auto _status = os::access(name.c_str(),os::access_mode_file_exists);
        if (_status != status_ok) {
            fprintf(stderr, "the file(%s) not found\n", name.c_str());
            return nullptr;
        }
        uint64 size;
        int fd = 0;
        //尝试生成文件描述符号
        maybe_error(os::open(name.c_str(), os::open_mode_read_write,0666,fd));
        //检查文件大小
        maybe_error(os::get_file_size(fd,size));
        //文件异常
        if (size < sizeof(journal_meta)) {
            os::close(fd);
            os::unlink(name.c_str());
            fprintf(stderr, "the file(%s) is too small,it is invalid\n", name.c_str());
            return nullptr;
        }
        //读取元数据
        maybe_error(os::read(fd,0,&meta,sizeof(meta)));
        //检验元数据魔数
        if (strncmp(meta.magic,journal_meta::magic_string,sizeof(journal_meta::magic_string)) != 0) {
            _status = status_invalid_journal;
            goto error_handle;
        }
        return std::unique_ptr<journal>(new journal(fd));
        error_handle:
            fprintf(stderr, "can't open file(%s),status = %d",name.c_str(),_status);
            if (fd > 0) {
                os::close(fd);
            }
            return nullptr;
    }

    std::unique_ptr<journal> journal::create(const std::string &name) {
        auto _status = os::access(name.c_str(),os::access_mode_file_exists);
        if (_status == status_ok) {
            fprintf(stderr, "the file(%s) is exists\n", name.c_str());
            return nullptr;
        }
        int fd = 0;
        //尝试生成文件描述符号
        maybe_error(os::open(name.c_str(), os::open_mode_read_write | os::open_mode_create,0666,fd));
        return std::unique_ptr<journal>(new journal(fd));
        error_handle:
            fprintf(stderr, "can't create file(%s),status = %d",name.c_str(),_status);
            if (fd > 0) {
                //如果创建wal过程中出现故障，则在关闭文件描述符后还需删除文件
                os::close(fd);
                os::unlink(name.c_str());
            }
            return nullptr;
    }


    int journal::begin_read_transaction() {
        assert(status == journal_status::normal);
        //读取元数据
        const auto _status = os::read(fd,0,&meta,sizeof(meta));
        if (_status == status_ok)
            status = journal_status::read_transaction;
        return _status;
    }
    uint64 journal::get_origin_file_size() const {
        assert(status == journal_status::read_transaction);
        return meta.origin_file_size;
    }

    uint64 journal::get_page_count() const {
        assert(status == journal_status::read_transaction);
        return meta.page_count;
    }
    int journal::reset() {
        assert(status == journal_status::read_transaction);
        read_page_count = 0;
        int64 unused;
        return os::seek(fd,sizeof(journal_meta),os::seek_mode_set,unused);
    }
    int journal::get_page(void *buffer,int64 & out_page_no) {
        if (read_page_count >= meta.page_count) {
            return status_file_read_complete;
        }
        os::io_vec iov[2];
        page_meta page{};
        iov[0].set(&page,sizeof(page));
        iov[1].set(buffer,page_size);
        const auto _status = os::readv(fd,iov,2);
        read_page_count++;
        out_page_no = page.page_no;
        if (_status == status_ok) {
            if (os::calculate_checksum(buffer,page_size) != page.checksum)
                return status_invalid_journal_page;
        }
        return _status;
    }

    int journal::begin_write_transaction(uint64 origin_file_size) {
        assert(status == journal_status::normal);
        read_page_count = 0;
        meta.page_count = 0;
        meta.origin_file_size = origin_file_size;
        memcpy(meta.magic,journal_meta::magic_string,sizeof(journal_meta::magic_string));
        auto _status = os::ftruncate(fd,0);
        if (_status != status_ok)
            return _status;
        _status = os::write(fd,0,&meta,sizeof(journal_meta));
        if (_status == status_ok) {
            status = journal_status::write_transaction;
        }
        return _status;
    }
    int journal::append(int64 page_no, void *buffer) {
        assert(status == journal_status::write_transaction);
        os::io_vec iov[2];
        page_meta page{};
        page.page_no = page_no;
        page.checksum = os::calculate_checksum(buffer,page_size);
        iov[0].set(&page,sizeof(page_meta));
        iov[1].set(buffer, page_size);
        const auto _status = os::writev(fd,iov,2);
        if (_status == status_ok) {
            meta.page_count++;
        }
        return _status;
    }
    int journal::commit_transaction() {
        assert(status == journal_status::write_transaction || status == journal_status::read_transaction);
        if (status == journal_status::read_transaction) {
            read_page_count = 0;
            status = journal_status::normal;
            return status_ok;
        }
        //提交写事务需将元数据写入日志头
        const auto  _status = os::write(fd,0,&meta,sizeof(journal_meta));
        if (_status == status_ok) {
            status = journal_status::normal;
        }
        return _status;
    }


}