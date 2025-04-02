//
// Created by user on 25-2-12.
//

#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <filesystem>

#include "utility.h"
#include "table.h"
#include "btree.h"
/*
 *  在数据库启动时调用，仅存在单个实例
 *
 */
namespace iedb
{
    class dbManager
    {
    public:
        struct dbTable
        {
            std::vector<uint8> data;
            std::unique_ptr<table> _table;
            std::unique_ptr<btree> tree;
            dbTable(std::vector<uint8>& data,std::unique_ptr<table>&_table,std::unique_ptr<btree>&tree);
            [[nodiscard]] const std::string& get_name() const{return _table->get_name();}
        };
        std::string root_dir;
        std::unordered_map<std::string,std::unique_ptr<dbTable>> map;

    private:
        static std::vector<std::filesystem::directory_entry> extract_dirs(const std::string & path);
        static std::unique_ptr<dbTable> open_table(const std::string&dir_path);
        static std::unique_ptr<dbTable> create_table(const std::string&root_dir,std::unique_ptr<table> & _table);
        void extract_parameters(void * json_root_node);
        void initialize_file_system();
        dbManager();

    public:
        static dbManager& get_instance();
        dbManager(const dbManager& init) = delete;
        constexpr const std::string& get_root_dir(){return root_dir;}
        static int get_dbTable(const std::string& table_name,dbTable*& out_table);
        static int create_dbTable(std::unique_ptr<table>& _table);
    };


}

#endif //INITIALIZER_H
