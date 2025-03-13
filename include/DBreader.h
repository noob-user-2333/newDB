//
// Created by user on 25-3-12.
//

#ifndef DBREADER_H
#define DBREADER_H
#include "newDB.h"
#include "table.h"
#include "utility.h"
struct DBreader {
private:
    struct row {
        uint64_t key;
        std::vector<iedb::column_value> values;
    };
    int pos;
    std::vector<row> rows;
public:
    DBreader() = default;
    int next();
    [[nodiscard]] int get_column_bytes(int column) const;
    int64_t get_column_int(int column);
    double get_column_double(int column);
    const char* get_column_string(int column);
    constexpr void append_row(uint64_t key,std::vector<iedb::column_value>&row_data) {rows.push_back({key, row_data});}

};


#endif //DBREADER_H
