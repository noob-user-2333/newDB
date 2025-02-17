//
// Created by user on 25-2-12.
//
#include "initializer.h"
#include "os.h"
#include "../third-part/cjson/cJSON.h"
namespace iedb
{
    void initializer::extract_parameters(void* json_root_node)
    {
        auto root = static_cast<cJSON*>(json_root_node);
        auto root_dir_node = cJSON_GetObjectItem(root, "DbRootDir");
        assert(root_dir_node && cJSON_IsString(root_dir_node) && root_dir_node->valuestring);
        root_dir = root_dir_node->valuestring;
    }
    void initializer::initialize_file_system()
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

    }

    initializer::initializer()
    {
        static char buffer[1024 * 1024];
        //读取配置文件到指定缓存
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
    const initializer& initializer::get_instance()
    {
        static initializer instance;
        return instance;
    }


}

