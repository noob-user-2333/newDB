//
// Created by user on 25-3-12.
//
#include "DBreader.h"

int DBreader::next() {
    pos++;
    if (pos >= rows.size())
        return iedb::status_out_of_range;
    return iedb::status_ok;
}

int DBreader::get_column_bytes(int column) const {
    const auto&[key, values] = rows.at(pos);
    if (values.at(column).index() != 2)
        return 8;
    return static_cast<int>(std::get<std::string>(values.at(column)).size());
}
double DBreader::get_column_double(int column) {
    const auto&[key, values] = rows.at(pos);
    return std::get<double>(values.at(column));
}
int64_t DBreader::get_column_int(int column) {
    const auto&[key, values] = rows.at(pos);
    return static_cast<int64_t>(std::get<uint64_t>(values.at(column)));
}
const char *DBreader::get_column_string(int column) {
    const auto&[key, values] = rows.at(pos);
    return std::get<std::string>(values.at(column)).c_str();
}
