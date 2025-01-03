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

namespace iedb {
    using int8 = char;
    using uint8 = unsigned char;
    using int16 = short;
    using uint16 = unsigned short;
    using int32 = int;
    using uint32 = unsigned int;
    using int64 = long;
    using uint64 = unsigned long;



    static constexpr int status_error= -1;
    static constexpr int status_ok = 0;
    static constexpr int status_invalid_argument = 1;
    static constexpr int status_no_space = 2;
    static constexpr int status_invalid_fd = 3;
    static constexpr int status_not_access  = 4;
    static constexpr int status_io_error = 5;
    static constexpr int status_argument_overflow = 6;
    static constexpr int status_invalid_journal_page = 7;
    static constexpr int status_not_find_column = 8;
    static constexpr int status_not_find_table = 9;
    static constexpr int status_column_exists = 10;
    static constexpr int status_file_exists = 11;
    static constexpr int status_invalid_checksum = 12;
    static constexpr int status_invalid_journal = 13;
    static constexpr int status_file_read_complete = 14;


    static constexpr int page_size = 32 * 1024;
    static constexpr uint64 current_format_version = 0x0001000;

};

#endif //UTILITY_H
