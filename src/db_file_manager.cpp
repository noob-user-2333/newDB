//
// Created by user on 24-12-24.
//
#include "db_file_manager.h"
#define maybe_error(x) {_status = x; if(_status != status_ok) goto error_handle;}

namespace iedb {

    struct journal_page_header {
        uint32 page_no;
        uint32 check_sum;
        journal_page_header(uint32 page_no,uint32 check_sum):page_no(page_no),check_sum(check_sum){}
    };

    int journal_header::check_header() const {
            if (memcmp(magic, magic_string, sizeof(magic_string)) == 0)
                return status_ok;
            return status_error;
    }

    int64 journal_header::get_journal_page_offset(uint32 page_no) {
        return static_cast<int64>(sizeof(journal_header) + page_no * (sizeof(journal_page_header) + page_size));
    }



    int db_file_manager::create_journal() {
        assert(status == file_status::transaction);
        auto _status = 0;
        maybe_error(os::open(journal_name.c_str(),os::open_mode_read_write | os::open_mode_create,0666,fd_journal));
        uint64 size;
        maybe_error(os::get_file_size(fd,size));
        header.origin_file_size = size;
        header.page_count = 0;
        maybe_error(os::write(fd_journal,0,&header,sizeof(journal_header)));
            return status_ok;
        error_handle:
        if (os::access(journal_name.c_str(),os::access_mode_file_exists) == status_ok)
            os::unlink(journal_name.c_str());
        return _status;
    }
    int db_file_manager::delete_journal(){
        assert(status == file_status::transaction);
        os::close(fd_journal);
        auto _status = os::unlink(journal_name.c_str());
        if (_status != status_ok)
            status = file_status::error;
        return _status;
    }
    int db_file_manager::backup_page(uint32 page_no,void *buffer) {
        assert(status == file_status::transaction);
        if (bitmap.contains(page_no))
            return status_ok;
        os::io_vec vec[2];
        vec[0].set(&header, sizeof(journal_page_header));
        vec[1].set(buffer, page_size);
        auto _status = 0;
        //修改日志头
        auto offset = journal_header::get_journal_page_offset(header.page_count);
        header.page_count++;
        maybe_error(os::seek(fd,offset,os::seek_mode_set,offset));
        maybe_error(os::writev(fd_journal,vec,2));
        maybe_error(os::write(fd_journal,0,&header,sizeof(journal_page_header)));
        maybe_error(os::fdatasync(fd_journal));
        return status_ok;
        error_handle:
            status = file_status::error;
            return _status;
    }

    db_file_manager::db_file_manager(std::string filename, int fd):fd_journal(0),filename(std::move(filename)),fd(fd),status(file_status::error),meta_page() {
    }
    db_file_manager::~db_file_manager() {
        os::close(fd);
    }


    std::unique_ptr<db_file_manager> db_file_manager::open(const char *name, bool create) {
        int mode = os::open_mode_read_write;
        if (create)
            mode |= os::open_mode_create;
        int fd;
        auto status = os::open(name,mode, 0666,fd);
        if (status != status_ok) {
            fprintf(stderr, "can't open file %s: %d\n", name, status);
            return nullptr;
        }
        auto db_file = new db_file_manager(name,fd);
        status = db_file->rollback_transaction();
        if (status != status_ok) {
            delete db_file;
            fprintf(stderr, "rollback failed after open file %s:%d",name,status);
            return nullptr;
        }
        return std::unique_ptr<db_file_manager>(db_file);
    }

    int db_file_manager::begin_transaction() {
        assert(status == file_status::normal);
        const auto _status = os::read(fd,0,meta_page,page_size);
        if (_status == status_ok)
            status = file_status::transaction;
        return _status;
    }
    int db_file_manager::commit_transaction() {
        assert(status == file_status::transaction);
        auto _status = delete_journal();
        if (_status!= status_ok) {
            status = file_status::error;
            return _status;
        }
        status = file_status::normal;
        return status_ok;
    }
    int db_file_manager::rollback_transaction() {
        return status_ok;
    }



}
