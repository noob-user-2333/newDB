//
// Created by user on 25-3-12.
//
#include "newDB.h"
#include "vdbe_cursor.h"
#include "vdbe_expr.h"
#include "AST.h"
#include "DBreader.h"
using namespace iedb;




static int execute_query(AST &ast, DBreader &reader) {
    //首先打开指定table
    auto cursor = vdbe_cursor::open(ast.target_table->to_string());
    if (!cursor)
        return iedb::status_not_found;
    auto& _table = cursor->get_table();
    uint64_t key;
    std::vector<uint8_t> record;
    std::vector<column_value> row;
    std::vector<std::unique_ptr<vdbe_expr>> exprs;
    //表达式树转化为后缀表达式
    for (auto node = ast.where;node;node = node->next)
        exprs.emplace_back(vdbe_expr::generate(_table, node));
    //遍历数据以进行筛选
    while (cursor->next() == iedb::status_ok) {
        cursor->get_record(key,record);
        cursor->record_to_row(record,row);
        // 若符合查询条件，则将数据添加到reader中
        auto condition_met = true;
        for (const auto&expr:exprs) {
            auto v = expr->run(row);
            if (vdbe_expr::is_zero(v)) {
                condition_met = false;
                break;
            }
        }
        if (condition_met)
            reader.append_row(key,row);
    }
    //开始针对group by进行分类
    exprs.clear();
    if (ast.group) {
        for (auto node = ast.group;node;node = node->next)
            exprs.emplace_back(vdbe_expr::generate(_table, node));
        //根据exprs运行结果进行分类
        std::unordered_map<std::vector<column_value>*,std::vector<column_value>*> map;

    }
    return status_ok;
}


int IEDB_execute_query_sql(const char *sql, DBreader *reader) {
    auto ast = iedb::AST::parse(sql);
    if (ast == nullptr && ast->type != iedb::token_type::select)
        return iedb::status_invalid_sql;
    return execute_query(*ast, *reader);
}
