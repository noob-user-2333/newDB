//
// Created by user on 24-11-26.
//
#ifndef UTILITY_H
#define UTILITY_H

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <stack>
#include <stdexcept>
#include <variant>
#include <list>
#include <array>
#include <optional>
namespace iedb {
    using int8 = char;
    using uint8 = unsigned char;
    using int16 = short;
    using uint16 = unsigned short;
    using int32 = int;
    using uint32 = unsigned int;
    using int64 = long;
    using uint64 = unsigned long;

    struct memory_slice
    {
        void* buffer;
        uint64 size;
        void set( void* buffer,uint64 size)
        {
            this->size = size;
            this->buffer = buffer;
        }
    };

    static constexpr int status_error= -1;
    static constexpr int status_ok = 0;
    static constexpr int status_invalid_argument = 1;
    static constexpr int status_no_space = 2;
    static constexpr int status_invalid_fd = 3;
    static constexpr int status_not_access  = 4;
    static constexpr int status_io_error = 5;
    static constexpr int status_argument_overflow = 6;
    static constexpr int status_invalid_journal_page = 7;
    static constexpr int status_not_found = 8;
    static constexpr int status_key_exists = 9;
    static constexpr int status_column_exists = 10;
    static constexpr int status_file_exists = 11;
    static constexpr int status_table_exists = 12;
    static constexpr int status_invalid_checksum = 13;
    static constexpr int status_invalid_journal = 14;
    static constexpr int status_invalid_data_type = 15;
    static constexpr int status_invalid_sql = 16;
    static constexpr int status_file_read_complete = 17;
    static constexpr int status_error_data_type = 18;
    static constexpr int status_out_of_range = 19;
    static constexpr int status_io_error_short_read = 20;
    static constexpr int status_file_not_exists = 21;
    static constexpr int status_error_file_type = 22;

    static constexpr int page_size = 64 * 1024 ;
    static constexpr uint64 current_format_version = 0x0001000;

    static constexpr char config_file_name[] = "settings.json";
};

#endif //UTILITY_H
