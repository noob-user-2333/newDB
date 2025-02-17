//
// Created by user on 25-1-2.
//
#include "journal.h"
#include "os.h"

namespace iedb {
#define maybe_error(x){_status = x; if(_status != status_ok) goto error_handle;}
    journal::journal(int fd):fd(fd),status(journal_status::normal)
    {}
    journal::~journal()
    {
        assert(status == journal_status::normal);
        os::close(fd);
    }

    std::unique_ptr<journal> journal::open(const std::string& path)
    {
        int fd;
        //检查文件是否存在
        bool exists = os::access(path.c_str(),os::access_mode_file_exists) == status_ok;
        auto status = os::open(path.c_str(),os::open_mode_create | os::open_mode_read_write,0666,fd);
        if (status != status_ok)
        {
            fprintf(stderr, "journal could not open %s: %s\n",path.c_str(),strerror(errno));
            return nullptr;
        }
        //对原本存在的文件需要校验日志头
        journal_meta meta{};
        if(exists)
        {
            status = os::read(fd,0,&meta,sizeof(journal_meta));
            if (status != status_ok)
            {
                fprintf(stderr, "journal could not read meta: %s\n",strerror(errno));
                return nullptr;
            }
            if (strcmp(meta.magic,journal_meta::magic_string) != 0)
            {
                fprintf(stderr,"journal::open: bad magic string\n");
                return nullptr;
            }
        }else
        {
            //对新创建的文件需写入日志头
            status = os::write(fd,&meta,sizeof(journal_meta));
            if (status != status_ok)
            {
                fprintf(stderr,"journal::open:  can not write header\n");
                return nullptr;
            }
        }
        return std::unique_ptr<journal>(new journal(fd));
    }

    void journal::add_page_to_buffer(int page_no, const void *buffer) {
        auto& page = page_buffer[page_buffer_count];
        const auto checksum = os::calculate_checksum(buffer,page_size);
        page.page_no = page_no;
        page.checksum = checksum;
        memcpy(page.buffer,buffer,page_size);
        page_buffer_count++;
    }
    int journal::commit_buffer(){
        assert(status == journal_status::write);
        //首先尝试提交缓存
        auto commit_page = page_buffer_count;
        auto offset = static_cast<int64>(sizeof(journal_meta) + meta.page_count * sizeof(journal_page));
        auto _status = os::write(fd,offset,page_buffer,sizeof(journal_page) * commit_page);
        if(_status != status_ok)
            return _status;
        //将新日志头写入文件
        meta.page_count += commit_page;
        _status = os::write(fd,0,&meta,sizeof(journal_meta));
        if(_status != status_ok)
            return _status;
        page_buffer_count = 0;
        return status_ok;
    }


    int journal::begin_read_transaction() {
        assert(status == journal_status::normal);
        //读取元数据
        auto _status = os::read(fd,0,&meta,sizeof(meta));
        if (_status == status_ok)
        {
            //检验是否为日志文件
            if (memcmp(meta.magic,journal_meta::magic_string,sizeof(journal_meta::magic_string)) != 0)
            {
                //如果两者不等，则说明其非日志文件
                return status_invalid_journal;
            }
            status = journal_status::read;
        }
        return _status;
    }
    int64 journal::get_origin_file_size() const {
        assert(status == journal_status::read);
        return meta.origin_file_size;
    }
    uint64 journal::get_page_count() const {
        assert(status == journal_status::read);
        return meta.page_count;
    }

    int journal::read_next_page(void *buffer,int & out_page_no) const {
        static journal_page page{};
        const auto _status = os::read(fd,&page,sizeof(journal_page));
        out_page_no = page.page_no;
        if (_status == status_ok) {
            if (os::calculate_checksum(buffer,page_size) != page.checksum)
                return status_invalid_journal_page;
        }
        memcpy(buffer,page.buffer,page_size);
        return _status;
    }

    int journal::begin_write_transaction(const int64 origin_file_size) {
        assert(status == journal_status::normal);
        //初始化日志头
        meta.page_count = 0;
        meta.origin_file_size = origin_file_size;
        memcpy(meta.magic,journal_meta::magic_string,sizeof(journal_meta::magic_string));
        auto _status = os::write(fd,0,&meta,sizeof(journal_meta));
        if (_status == status_ok) {
            status = journal_status::write;
            page_buffer_count = 0;
            bitmap.clear();
        }
        return _status;
    }
    int journal::add_page(int page_no, const void *buffer) {
        assert(status == journal_status::write);
        //先确定是否有必要记录该页面
        auto offset = static_cast<int64>(page_no) * page_size;
        if (offset >= page_size)
            return status_ok;
        //先确定是否需要提交缓存
        //需注意，元数据仅在开始或提交事务时进行修改
        if (page_buffer_count >= max_buffer_page) {
            const auto _status = commit_buffer();
            if (_status != status_ok)
                return _status;
        }
        if (bitmap.contains(page_no) == false) {
            add_page_to_buffer(page_no,buffer);
            bitmap.add(page_no);
        }
        return status_ok;
    }
    int journal::commit_transaction() {
        assert(status == journal_status::write || status == journal_status::read);
        auto _status = status_ok;
        // 提交写事务分两步:
        //          1 提交缓存到日志文件
        //          2 将日志头提交到日志文件
        if (status == journal_status::read)
            status = journal_status::normal;
        if (status == journal_status::write)
        {
            _status = commit_buffer();
            if (_status == status_ok) {
                //缓存提交后将新的日志头写入
                _status = os::write(fd,0,&meta,sizeof(journal_meta));
                status = journal_status::normal;
            }
        }
        return _status;
    }
    int journal::clear()
    {
        assert(status == journal_status::normal);
        //用于在完成数据库写入或检测到日志文件损坏时清空日志
        meta.page_count = 0;
        meta.origin_file_size = 0;
        const auto _status = os::write(fd,0,&meta,sizeof(journal_meta));
        return _status;
    }



    int journal::close()
    {
        assert(status == journal_status::write || status == journal_status::read);
        auto _status = os::close(fd);
        if (_status != status_ok)
            return _status;
        status = journal_status::normal;
        return status_ok;
    }



}