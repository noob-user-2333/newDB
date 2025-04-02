//
// Created by user on 25-3-12.
//
#include "newDB.h"
#include "vdbe_cursor.h"
#include "vdbe_expr.h"
#include "AST.h"
#include "DBreader.h"
using namespace iedb;

static std::vector<std::vector<column_value> > order_expr_compute(const std::vector<table::row> &rows,
                                                                  std::vector<std::unique_ptr<vdbe_expr> > &exprs) {
    auto count = rows.size();
    auto expr_count = exprs.size();
    std::vector<std::vector<column_value> > ret;
    for (auto i = 0; i < count; i++) {
        std::vector<column_value> compute_result(expr_count + 1);
        compute_result[0] = static_cast<int64>(i);
        for (auto j = 0; j < expr_count; j++)
            compute_result[j + 1] = exprs.at(j)->run(rows[i].values);
        ret.push_back(std::move(compute_result));
    }
    return std::move(ret);
}

static int order_sort(const std::vector<column_value> &a, const std::vector<column_value> &b) {
    assert(a.size() == b.size());
    auto count = a.size();
    for (auto i = 1; i < count; i++) {
        assert(a[i].index() == b[i].index());
        if (a[i] == b[i])
            continue;
        return a[i] < b[i];
    }
    return a[0] < b[0];
}

static int key_sort(const table::row &row1, const table::row &row2) {
    return row1.key < row2.key;
}

static int execute_query(AST &ast, DBreader &reader) {
    //首先打开指定table
    auto cursor = vdbe_cursor::open(ast.target_table->to_string());
    if (!cursor)
        return iedb::status_not_found;
    cursor->begin_read();
    auto &_table = cursor->get_table();
    uint64_t key;
    std::vector<uint8_t> record;
    table::row row;
    std::vector<std::unique_ptr<vdbe_expr> > exprs;
    std::vector<table::row> rows;
    //表达式树转化为后缀表达式
    for (auto node = ast.where; node; node = node->next)
        exprs.emplace_back(vdbe_expr::generate(_table, node));
    //遍历数据以进行筛选
    while (cursor->next() == iedb::status_ok) {
        cursor->get_record(row.key, record);
        cursor->record_to_row(record, row.values);
        // 若符合查询条件，则将数据添加到reader中
        auto condition_met = true;
        for (const auto &expr: exprs) {
            auto v = expr->run(row.values);
            if (vdbe_expr::is_zero(v)) {
                condition_met = false;
                break;
            }
        }
        if (condition_met)
            rows.emplace_back(row);
    }
    //开始针对group by进行分类
    exprs.clear();
    if (ast.group) {
        for (auto node = ast.group; node; node = node->next)
            exprs.emplace_back(vdbe_expr::generate(_table, node));
        std::unordered_map<uint64, const table::row *> map1, map2;
        auto hash = vdbe_expr::get_Hash();
        //遍历数据
        for (const auto &_row: rows) {
            const auto &row_data = _row.values;
            hash->reset();
            for (const auto &expr: exprs)
                expr->update_hash(*hash, row_data);
            auto hash_value = hash->get_hash();
            //检查是否存在相同hash的行数据
            auto it = map1.find(hash_value);
            if (it == map1.end()) {
                map1[hash_value] = &_row;
            } else {
                //如果存在同hash的数据，则先确定二者group by结果是否一致
                auto same = true;
                for (auto &expr: exprs) {
                    auto v1 = expr->run(it->second->values);
                    auto v2 = expr->run(row_data);
                    if (v1 != v2) {
                        same = false;
                        break;
                    }
                }
                if (same == false) {
                    //如果不一致则尝试将该数据移送到map2
                    it = map2.find(hash_value);
                    //正常情况下不应当存在
                    if (it != map2.end())
                        throw std::runtime_error("对于三个group by运算结果不同的行，xxhash计算出相同hash值");
                    map2[hash_value] = &_row;
                }
            }
        }
        //获取有效行
        std::vector<table::row> new_rows;
        for (auto valid_row: map1)
            new_rows.emplace_back(*valid_row.second);
        for (auto valid_row: map2)
            new_rows.emplace_back(*valid_row.second);
        rows.swap(new_rows);
        //分类后数据排列顺序可能与key不符，整理
        std::sort(rows.begin(), rows.end(), key_sort);
    }
    exprs.clear();
    if (ast.order) {
        for (auto node = ast.order; node; node = node->next)
            exprs.emplace_back(vdbe_expr::generate(_table, node));
        auto result = order_expr_compute(rows, exprs);
        std::sort(result.begin(), result.end(), order_sort);
        std::vector<table::row> new_row;
        for (const auto &row_result: result) {
            auto index = std::get<int64>(row_result[0]);
            new_row.emplace_back(rows[index]);
        }
        rows.swap(new_row);
    }
    //对于select *，exprs中无元素以进行区别
    exprs.clear();

    if (ast.master->type != token_type::star) {
        for (auto node = ast.master; node; node = node->next)
            exprs.emplace_back(vdbe_expr::generate(_table, node));
    }
    reader.swap(rows, exprs);
    return cursor->commit();
}


int IEDB_execute_query_sql(const char *sql, DBreader **reader) {
    auto ast = iedb::AST::parse(sql);
    if (ast == nullptr && ast->type != iedb::token_type::select)
        return iedb::status_invalid_sql;
    *reader = new DBreader();
    return execute_query(*ast, **reader);
}
