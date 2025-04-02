//
// Created by user on 25-3-12.
//
#include "DBreader.h"

int DBreader::next() {
    if (have_read) {
        ++it;
    } else {
        it = rows.begin();
        have_read = true;
    }
    if (it == rows.end())
        return iedb::status_out_of_range;
    values.clear();
    if (exprs.empty()) {
        values = it->values;
    }else {
        for (const auto &expr: exprs) {
            values.emplace_back(expr->run(it->values));
        }
    }
    return iedb::status_ok;
}

int DBreader::prev() {
    assert(have_read);
    if (it == rows.begin())
        return iedb::status_out_of_range;
    --it;
    values.clear();
    if (exprs.empty()) {
        values = it->values;
    }else {
        for (const auto &expr: exprs) {
            values.emplace_back(expr->run(it->values));
        }
    }
    return iedb::status_ok;
}

const iedb::column_value &DBreader::get_column(int column) const {
    return values.at(column);
}

int DBreader::get_column_bytes(int column) const {
    auto &values = get_column(column);
    if (values.index() != 2)
        return 8;
    return static_cast<int>(std::get<std::string>(values).size());
}

double DBreader::get_column_double(int column) const {
    auto &values = get_column(column);
    if (values.index() == 0)
        return static_cast<double>(std::get<int64_t>(values));
    return std::get<double>(values);
}

int64_t DBreader::get_column_int(int column) const {
    auto &values = get_column(column);
    if (values.index() == 1)
        return static_cast<int64_t>(std::get<double>(values));
    return std::get<int64_t>(values);
}

const char *DBreader::get_column_string(int column) const {
    auto &values = get_column(column);
    return std::get<std::string>(values).c_str();
}
