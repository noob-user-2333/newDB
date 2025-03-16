//
// Created by user on 25-3-12.
//

#ifndef DBREADER_H
#define DBREADER_H
#include "newDB.h"
#include "table.h"
#include "utility.h"
#include "vdbe_expr.h"

struct DBreader {
private:

    std::vector<iedb::table::row> rows;
    std::vector<iedb::column_value> values;
    std::vector<std::unique_ptr<iedb::vdbe_expr>> exprs;
    std::vector<iedb::table::row>::iterator it;
    bool have_read;
    [[nodiscard]] const iedb::column_value& get_column(int column)const ;
public:
    DBreader() = default;
    int next();
    int prev();
    [[nodiscard]] int get_column_bytes(int column) const;
    [[nodiscard]] int64_t get_column_int(int column) const;
    [[nodiscard]] double get_column_double(int column) const;
    [[nodiscard]] const char* get_column_string(int column) const;
    constexpr void swap(std::vector<iedb::table::row>&rows,std::vector<std::unique_ptr<iedb::vdbe_expr>>&exprs) {
        this->rows.swap(rows);
        this->exprs.swap(exprs);
        have_read = false;
    }

};


#endif //DBREADER_H
