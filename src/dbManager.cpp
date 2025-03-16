//
// Created by user on 25-2-12.
//
#include "dbManager.h"
#include "os.h"
#include "../third-part/cjson/cJSON.h"
#include <filesystem>
#include <iostream>

namespace iedb
{
    dbManager::dbTable::dbTable(std::vector<uint8>& data, std::unique_ptr<table>& _table, std::unique_ptr<btree>& tree):
    data(std::move(data)),_table(std::move(_table)),tree(std::move(tree)){}

    void dbManager::extract_parameters(void* json_root_node)
    {
        auto root = static_cast<cJSON*>(json_root_node);
        auto root_dir_node = cJSON_GetObjectItem(root, "DbRootDir");
        assert(root_dir_node && cJSON_IsString(root_dir_node) && root_dir_node->valuestring);
        root_dir = root_dir_node->valuestring;
    }

    std::vector<std::filesystem::directory_entry> dbManager::extract_dirs(const std::string& path)
    {
        std::vector<std::filesystem::directory_entry> dirs;
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                dirs.emplace_back(entry);
            }
        }
        return std::move(dirs);
    }

    std::unique_ptr<dbManager::dbTable> dbManager::open_table(const std::string& dir_path)
    {
        auto data_path = dir_path + "/data";
        auto table_path = dir_path + "/table";
        //打开对应文件
        auto _pager = pager::open(data_path);
        if (_pager == nullptr)
        {
            fprintf(stderr,"can't open file:%s\n",data_path.c_str());
            return nullptr;
        }
        auto size = 0L;
        auto fd = 0;
        auto status = os::open(table_path.c_str(),os::open_mode_read,0,fd);
        if (status != status_ok)
        {
            fprintf(stderr,"can't open file:%s\n",data_path.c_str());
            return nullptr;
        }
        status = os::get_file_size(fd,size);
        if (status != status_ok)
        {
            fprintf(stderr,"can't get file size:%s\n",data_path.c_str());
            return nullptr;
        }
        std::vector<uint8> data(size);
        status = os::read(fd,0,data.data(),size);
        if (status != status_ok)
        {
            fprintf(stderr,"can't read the file:%s\n",data_path.c_str());
            return nullptr;
        }
        auto _table = table::get(data.data(),data.size());
        auto tree = btree::open(_pager);
        return std::make_unique<dbTable>(data, _table, tree);
    }
    std::unique_ptr<dbManager::dbTable> dbManager::create_table(const std::string& root_dir,std::unique_ptr<table>& _table)
    {
        auto dir_path = root_dir +"/"+ _table->get_name();
        auto data_path = dir_path + "/data";
        auto table_path = dir_path + "/table";
        std::vector<uint8> data;
        table::translate_to_buffer(*_table,data);
        auto fd = 0;
        //创建目录
        auto status = os::mkdir(dir_path.c_str());
        if (status != status_ok)
        {
            fprintf(stderr,"can't create dir:%s\n",dir_path.c_str());
            return nullptr;
        }
        //创建文件与写入
        status = os::open(table_path.c_str(),os::open_mode_write | os::open_mode_create,0666,fd);
        if (status != status_ok)
        {
            fprintf(stderr,"can't create file:%s\n",table_path.c_str());
            return nullptr;
        }
        status = os::write(fd,0,data.data(),data.size());
        if (status!= status_ok)
        {
            fprintf(stderr,"can't write to file:%s\n",table_path.c_str());
            return nullptr;
        }
        auto _page = pager::open(data_path);
        if (_page == nullptr)
        {
            fprintf(stderr,"can't open file:%s\n",data_path.c_str());
            return nullptr;
        }
        auto tree = btree::open(_page);
        return std::make_unique<dbTable>(data, _table, tree);
    }



    void dbManager::initialize_file_system()
    {
        auto status = os::is_directory(root_dir.c_str());
        //判断指定根目录是否为目录
        if (status == status_error_file_type)
        {
            char buffer[10240];
            sprintf(buffer,"the file of %s is not directory",root_dir.c_str());
            throw std::runtime_error(buffer);
        }
        //如果不存在该目录则需创建目录
        if (status == status_file_not_exists)
        {
            assert(os::mkdir(root_dir.c_str()) == status_ok);
        }
        //扫描根目录下所有目录，确定存在哪些数据表
        auto dirs = extract_dirs(root_dir);
        for (auto& dir:dirs) {
            auto p = open_table(dir.path().c_str());
            if (p)
                map[dir.path().filename().generic_string()] = std::move(p);
        }
    }

    dbManager::dbManager()
    {
        static char buffer[1024 * 1024];
        //读取配置文件到指定缓存
        std::cout << "current work dir:" << std::filesystem::current_path().generic_string() << std::endl;
        int fd;
        auto status = os::open(config_file_name,os::open_mode_read,0,fd);
        assert(status == status_ok);
        status = os::read(fd, 0,buffer, sizeof(buffer));
        assert(status==status_io_error_short_read || status == status_ok );
        os::close(fd);
        //对缓存进行解析，获取根节点
        auto root = cJSON_Parse(buffer);
        assert(root);
        //根据根节点获取各个配置参数
        extract_parameters(root);
        cJSON_Delete(root);
        //初始化数据库文件系统
        initialize_file_system();
    }
    dbManager& dbManager::get_instance()
    {
        static dbManager instance;
        return instance;
    }
    int dbManager::get_dbTable(const std::string& table_name,dbTable*& out_table)
    {
        auto&manager = dbManager::get_instance();
        auto it = manager.map.find(table_name);
        if (it == manager.map.end())
            return status_not_found;
        out_table = (it->second.get());
        return status_ok;
    }
    int dbManager::create_dbTable(std::unique_ptr<table>& _table)
    {
        auto& manager = dbManager::get_instance();
        auto p  = create_table(manager.root_dir,_table);
        if (p)
        {
            manager.map.emplace(p->get_name(),std::move(p));
            return status_ok;
        }
        return status_error;
    }



}

