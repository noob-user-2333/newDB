//
// Created by user on 24-12-25.
//
#include "db_file_manager.h"


#define maybe_error(x) {_status = x; if(_status != status_ok) goto error_handle;}
namespace iedb {

    struct wal_record_header {
        int64 offset_in_origin;
        uint64 record_size;
        uint64 check_sum;
        wal_record_header(int64 offset,uint64 record_size,uint64 check_sum):offset_in_origin(offset),record_size(record_size),check_sum(check_sum){}
        wal_record_header() = default;
    };

    int wal_header::check_header() const {
            if (memcmp(magic, magic_string, sizeof(magic_string)) == 0)
                return status_ok;
            return status_invalid_checksum;
    }

    int db_file_manager::wal_create() {
        auto _status = 0;
        //创建日志文件并初始化文件头
        maybe_error(os::open(wal_name.c_str(),os::open_mode_read_write | os::open_mode_create | os::open_mode_excl,0666,fd_wal));
        wal_header.record_count = 0;
        wal_header.wal_file_size = sizeof(wal_header);
        maybe_error(os::write(fd_wal,0,&wal_header,sizeof(wal_header)));
            return status_ok;
        error_handle:
            return _status;
    }
    int db_file_manager::wal_delete(){
        auto _status = os::close(fd_wal);
        if (_status!= status_ok)
            return _status;
        fd_wal = 0;
        _status = os::unlink(wal_name.c_str());
        if (_status != status_ok)
            status = file_status::error;
        return _status;
    }
    int db_file_manager::wal_commit(){
        static std::vector<uint8> buffer;
        auto _status = 0;
        int64 offset;
        os::io_vec vec[2];
        wal_record_header record_header{};
        //如果当前为error状态，则需重新读取文件头
        if (status == file_status::error)
            maybe_error(os::read(fd_wal,0,&wal_header,sizeof(wal_header)));
        //其他与正常提交修改相同
        vec[0].set(&record_header,sizeof(record_header));
        vec[1].set(buffer.data(),0);
        maybe_error(os::seek(fd_wal,os::seek_mode_set,sizeof(wal_header),offset));
        for (auto i = 0;i < wal_header.record_count;i++) {
            //读取记录头
            maybe_error(os::readv(fd_wal,vec,1));
            //准备读取记录
            vec[1].iov_len = record_header.record_size;
            //如果buffer大小不足需要调整大小
            if (buffer.size() < record_header.record_size)
                buffer.resize(record_header.record_size);
            //读取记录
            maybe_error(os::readv(fd_wal,vec+1,1));
            //检查并验证记录
            if (os::compute_checksum(buffer.data(),record_header.record_size) != record_header.check_sum) {
                fprintf(stderr, "wal record checksum error\n");
                _status = status_invalid_checksum;
                goto error_handle;;
            }
            //如果验证通过，将数据写入原文件
            maybe_error(os::write(fd,record_header.offset_in_origin,buffer.data(),record_header.record_size));
        }
        maybe_error(os::fdatasync(fd));
            return status_ok;
        error_handle:
            return _status;
    }

    int db_file_manager::wal_add_record(int64 offset_in_origin, void *record, uint64 record_size) {
        wal_record_header page_header(offset_in_origin,record_size,os::compute_checksum(record,record_size));
        os::io_vec vec[2];
        vec[0].set(&page_header,sizeof(page_header));
        vec[1].set(record,record_size);
        const auto _status = os::writev(fd_wal,vec,2);
        if (_status == status_ok) {
            wal_header.record_count++;
            wal_header.wal_file_size += sizeof(wal_record_header) + record_size;
            return status_ok;
        }
        status = file_status::error;
        return _status;
    }


    int db_file_manager::wal_recovery() {
        int _status = os::access(wal_name.c_str(),os::access_mode_file_exists);
        if (_status == status_ok)
            return status_ok;
        //如果存在wal文件，则需要根据wal文件重新提交写入到原文件
        maybe_error(os::open(wal_name.c_str(),os::open_mode_read_write,0666,fd_wal));
        maybe_error(os::read(fd_wal,0,&wal_header,sizeof(wal_header)));
        maybe_error(wal_commit());
        maybe_error(wal_delete());
        return status_ok;
        error_handle:
            return _status;

    }








}