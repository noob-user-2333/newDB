//
// Created by user on 25-2-12.
//
#include "pager.h"
#include "os.h"

namespace iedb
{
    pager::dbPage::dbPage(pager& owner, int page_no):owner(owner),page_no(page_no),writable(false),data()
    {
        data = std::make_unique<std::array<uint8,page_size>>();
    }
    void *pager::dbPage::get_data() const {return data->data();}
    int pager::dbPage::get_page_no() const{return page_no;}

    int pager::dbPage::enable_write()
    {
        if (writable)
            return status_ok;
        auto status =  owner.mark_page_writable(*this);
        if (status == status_ok)
            writable = true;
        return status;
    }

    pager::pager(int fd, std::unique_ptr<journal>& j,int64 original_file_size):j(std::move(j)),fd(fd),page_count(original_file_size / page_size),status(pager_status::error)
    {}
    pager::~pager()
    {
        os::close(fd);
    }
    int pager::mark_page_writable(dbPage& page)
    {
        //在日志文件中登记该页面
        assert(status == pager_status::write_transaction);
        auto _status = j->add_page(page.page_no,page.data->data());
        if (_status !=status_ok)
        {
            status = pager_status::error;
            return _status;
        }
        //在writable_pages中添加该页面
        writable_pages.push_back(&page);
        return _status;
    }

    std::unique_ptr<pager> pager::open(const std::string& path)
    {
        auto journal_path = path + "-journal";
        //打开对应日志文件
        auto j = journal::open(journal_path);
        if (j == nullptr)
        {
            fprintf(stderr,"can not open the journal:%s\n",journal_path.c_str());
            return nullptr;
        }
        //打开对应数据文件并获取文件大小
        int fd;
        int64 original_file_size;
        auto status = os::open(path.c_str(),os::open_mode_create | os::open_mode_read_write,0666,fd);
        if (status != status_ok)
        {
            fprintf(stderr,"can't open the data file:%s\n",path.c_str());
            return nullptr;
        }
        //确保对应文件至少存在0号页
        status = os::fallocate(fd,0,page_size);
        if (status!= status_ok)
        {
            os::close(fd);
            fprintf(stderr,"can't fallocate the file(%s) when open it as pager\n",path.c_str());
            return nullptr;
        }
        status = os::get_file_size(fd,original_file_size);
        if (status != status_ok)
        {
            os::close(fd);
            fprintf(stderr,"can't get the file size:%s\n",path.c_str());
            return nullptr;
        }
        auto p = new pager(fd, j, original_file_size);
        if (p->rollback_back() == status_ok)
            return std::move(std::unique_ptr<pager>(p));
        return nullptr;
    }

    int pager::get_page(int page_no, dbPage_ref& out_page)
    {
        auto p = std::make_unique<dbPage>(*this,page_no);
        auto& page = pages.emplace_back(std::move(p));
        auto offset = static_cast<int64>(page_no) * page_size;
        if (page_no >= page_count)
        {
            out_page.reset();
            return status_out_of_range;
        }
        //先查找是否已缓存该页面
        auto it = map.find(page_no);
        if (it != map.end())
        {
            out_page = *it->second;
            return status_ok;
        }
        //未缓存则从文件中读取数据
        auto _status = os::read(fd,offset,page->get_data(),page_size);
        if (_status !=status_ok)
            return _status;
        out_page = *page;
        map.insert({page_no,page.get()});
        return status_ok;
    }
    int pager::get_new_page(dbPage_ref& out_page)
    {
        assert(status == pager_status::write_transaction);
        auto p = std::make_unique<dbPage>(*this,page_count);
        auto& page = pages.emplace_back(std::move(p));
        page_count++;
        out_page = *page;
        map.insert({page->get_page_no(),page.get()});
        page->enable_write();
        return status_ok;
    }



    int pager::begin_write_transaction()
    {
        assert(status == pager_status::normal);
        /*
         *  每次开启写事务核心在于需对日志文件进行初始化
         */
        auto _status = j->begin_write_transaction(page_count * page_size);
        if (_status != status_ok)
            return _status;
        status = pager_status::write_transaction;
        return status_ok;
    }

    /*
     * 一阶段提交分为两步:
     *      1、提交日志文件
     *      2、将缓存的被标记为writable的页面全部写入数据文件
     */
    int pager::commit_phase_one()
    {
        assert(status == pager_status::write_transaction);
        //提交日志
        auto _status = j->commit_transaction();
        if (_status != status_ok)
        {
            status = pager_status::error;
            return _status;
        }
        //提交缓存页面
        //先尝试直接扩展文件大小
        auto file_size = page_count * page_size;
        _status = os::fallocate(fd,0,file_size);
        if (_status != status_ok)
        {
            status = pager_status::error;
            return _status;
        }
        //写入页面
        for (auto  writable_page : writable_pages)
        {
            auto offset = static_cast<int64>(writable_page->page_no) * page_size;
            _status = os::write(fd,offset,writable_page->data->data(), page_size);
            if (_status != status_ok)
            {
                status = pager_status::error;
                return _status;
            }
        }
        _status = os::fdatasync(fd);
        if (_status != status_ok)
            status = pager_status::error;
        else
            status = pager_status::commit;
        return _status;
    }
     /*
     * 二阶段提交仅用于清空日志文件
     */
    int pager::commit_phase_two()
    {
        assert(status == pager_status::commit);
        auto _status = j->clear();
        if (_status != status_ok)
        {
            status = pager_status::error;
            return _status;
        }
        status = pager_status::normal;
        return status_ok;
    }
    /*
     * 故障恢复操作：
     *      逐步读取日志文件内容并写入
     *      清空日志
     */
    int pager::rollback_back()
    {
        assert(status == pager_status::error);
        auto _status = j->begin_read_transaction();
        if (_status != status_ok)
            return _status;
        //判断日志是否存在页面，即是否需要回滚
        auto count = j->get_page_count();
        if (count == 0)
        {
            status = pager_status::normal;
            j->commit_transaction();
            return status_ok;
        }
        //将原文件截断到初始长度
        auto original_size = j->get_origin_file_size();
        _status = os::ftruncate(fd,original_size);
        if (_status != status_ok)
            return _status;
        //遍历日志文件
        int page_no;
        for (auto i = 0; i < count; i++)
        {
            char buffer[page_size];
            _status = j->read_next_page(buffer,page_no);
            if (_status != status_ok)
                return _status;
            //在数据文件中写入该页面
            auto offset = static_cast<int64>(page_no) * page_size;
            _status = os::write(fd,offset,buffer, page_size);
            if (_status!= status_ok)
                return _status;
        }
        _status = os::fdatasync(fd);
        if (_status != status_ok)
            return _status;
        //清空日志文件
        j->commit_transaction();
        _status = j->clear();
        if (_status != status_ok)
            return _status;
        page_count = original_size / page_size;
        status = pager_status::normal;
        return status_ok;
    }
    void pager::release_buffer()
    {
        assert(status == pager_status::normal);
        map.clear();
        writable_pages.clear();
        pages.clear();
    }

}