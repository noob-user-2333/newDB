//
// Created by user on 25-3-12.
//
#include "newDB.h"

#include <chrono>

#include "AST.h"
#include "DBreader.h"
#include "vdbe.h"
#include "../backup/vdbe_call.h"
using namespace iedb;

static uint64 get_time() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    auto time_us = static_cast<uint64>(duration.count());
    return time_us;
}

static int create_sql_execute(iedb::AST &ast) {
    auto _table = iedb::table::create_new(ast.target_table->to_string());
    if (!_table)
        return iedb::status_table_exists;
    for (auto node = ast.master; node; node = node->next) {
        auto name_node = node;
        auto type_node = node->left;
        auto status = _table->add_column(name_node->to_string(),
                                         iedb::table::translate_token_to_column_type(type_node->type), 1);
        if (status != iedb::status_ok)
            return status;
    }
    return iedb::vdbe_cursor::create_table(*_table);
}

static column_value token_to_column_value(token *t) {
    switch (t->type) {
        case token_type::string: {
            return t->to_string();
        }
        case token_type::number_int: {
            char *end;
            auto num = std::strtol(t->sql + t->offset, &end, 10);
            assert(end == t->sql + t->offset + t->len);
            return static_cast<int64>(num);
        }
        case token_type::number_float: {
            char *end;
            auto num = std::strtod(t->sql + t->offset, &end);
            assert(end == t->sql + t->offset + t->len);
            return num;
        }
        case token_type::hex: {
            char *end;
            auto num = std::strtol(t->sql + t->offset + 2, &end, 16);
            assert(end == t->sql + t->offset + t->len);
            return static_cast<int64>(num);
        }
        default:
            throw std::runtime_error("should never happen");
    }
}

static int insert_sql_execute(iedb::AST &ast) {
    auto cursor = vdbe_cursor::open(ast.target_table->to_string());
    if (!cursor)
        return iedb::status_not_found;
    auto key = get_time();
    std::vector<column_value> row;
    for (auto node = ast.master; node; node = node->next) {
        row.emplace_back(token_to_column_value(node));
    }
    std::vector<uint8> buffer;
    cursor->row_to_record(row, buffer);
    return cursor->insert(key, buffer.data(), static_cast<int>(buffer.size()));
}

static int IEDB_execute_not_select(AST &ast) {
    switch (ast.type) {
        case iedb::token_type::create:
            return create_sql_execute(ast);
        case iedb::token_type::insert:
            return insert_sql_execute(ast);
        default:
            return iedb::status_invalid_sql;
    }
}


int IEDB_execute_sql_without_reader(const char *sql) {
    auto ast = iedb::AST::parse(sql);
    if (ast == nullptr)
        return iedb::status_invalid_sql;
    return IEDB_execute_not_select(*ast);
}

DBreader *IEDB_reader_malloc() {
    return new DBreader();
}

void IEDB_reader_free(const DBreader *reader) {
    delete reader;
}

int IEDB_reader_next(DBreader *reader) {
    return reader->next();
}

int IEDB_reader_get_column_bytes(DBreader *reader, int column) {
    return reader->get_column_bytes(column);
}

int64_t IEDB_reader_get_column_int(DBreader *reader, int column) {
    return reader->get_column_int(column);
}

double IEDB_reader_get_column_double(DBreader *reader, int column) {
    return reader->get_column_double(column);
}

const char *IEDB_reader_get_column_string(DBreader *reader, int column) {
    return reader->get_column_string(column);
}

