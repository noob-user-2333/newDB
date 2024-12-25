//
// Created by user on 24-12-24.
//
#include "db_file_manager.h"
#define maybe_error(x) {_status = x; if(_status != status_ok) goto error_handle;}

namespace iedb {

    db_file_manager::db_file_manager(std::string filename, int fd):fd_wal(0),filename(std::move(filename)),
    fd(fd),status(file_status::normal),file_header(),wal_name() {
        wal_name = this->filename + "-wal";
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
        //检查是否存在wal日志文件，如存在则进入error状态
        status = os::access(db_file->wal_name.c_str(),os::access_mode_file_exists);
        if (status != status_ok) {
            db_file->status = file_status::error;
            status = db_file->wal_commit();
            if (status!= status_ok) {
                fprintf(stderr, "can't commit wal file %s\n", db_file->wal_name.c_str());
                delete db_file;
                return nullptr;
            }
        }
        return std::unique_ptr<db_file_manager>(db_file);
    }

    int db_file_manager::begin_write_transaction() {
        assert(status == file_status::normal);
        auto _status = os::read(fd,0,&file_header,sizeof(file_header));
        if (_status == status_ok) {
            _status = wal_create();
            if (_status == status_ok)
                status = file_status::transaction_write;
        }
        return _status;
    }
    int db_file_manager::begin_read_transaction() {
        assert(status == file_status::normal);
        auto _status = os::read(fd,0,&file_header,sizeof(file_header));
        if (_status == status_ok)
            status = file_status::transaction_read;
        return _status;
    }

    int db_file_manager::commit_transaction() {
        assert(status == file_status::transaction_write || status == file_status::transaction_read);
        if (status == file_status::transaction_write) {
            auto _status = wal_commit();
            if (_status!= status_ok) {
                status = file_status::error;
                return _status;
            }
            wal_delete();
        }
        status = file_status::normal;
        return status_ok;
    }

    int db_file_manager::append(void *buffer, uint64 size) {
        assert(status == file_status::normal);
        auto _status = 0;
        maybe_error(os::read(fd,0,&file_header,sizeof(file_header)));
        maybe_error(os::write(fd,static_cast<int64>(file_header.file_size),buffer,size));
        file_header.file_size += size;
        file_header.record_count++;
        maybe_error(os::write(fd,0,&file_header,sizeof(file_header)));
        return status_ok;
        error_handle:
            return _status;
    }

    int db_file_manager::read(int64 offset, void *buffer, uint64 size) {
        assert(status == file_status::transaction_read);
        if (offset + size > file_header.file_size)
            return status_invalid_argument;
        return os::read(fd,offset,&buffer,size);
    }

    int db_file_manager::write(int64 offset, void *buffer, uint64 size) {
        assert(status == file_status::transaction_write);
        if (offset > file_header.file_size)
            return status_invalid_argument;
        return wal_add_record(offset,buffer,size);
    }

}
