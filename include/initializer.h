//
// Created by user on 25-2-12.
//

#ifndef INITIALIZER_H
#define INITIALIZER_H

#include "utility.h"

/*
 *  在数据库启动时调用，仅存在单个实例
 *
 */
namespace iedb
{
    class initializer
    {
    private:
        std::string root_dir;


        void extract_parameters(void * json_root_node);
        void initialize_file_system();
        initializer();

    public:
        initializer(const initializer& init) = delete;
        static const initializer& get_instance();

    };


}

#endif //INITIALIZER_H
