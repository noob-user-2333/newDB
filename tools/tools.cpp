//
// Created by user on 25-4-1.
//
#include <cassert>
#include <format>
#include "dbManager.h"
#include "AST.h"
#include "tool.h"
#include "utility.h"


namespace iedb {
    void ImportDataFromSqlite(sqlite3 *db) {
        static char buffer[4096];
        static char queryMasterTable[] = "SELECT sql FROM sqlite_master;";
        sqlite3_stmt *stmt;
        assert(sqlite3_prepare_v2(db,queryMasterTable,sizeof(queryMasterTable),&stmt,nullptr) == SQLITE_OK);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto sql = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            assert(sql);
            std::string creatSql = sql;
            creatSql.append(";");
            //先解析后建表
            auto ast = AST::parse(creatSql.c_str());
            assert(ast);
            auto res = IEDB_execute_sql_without_reader(creatSql.c_str());
            assert(res == status_ok);
            //开始查询sqlite数据
            sqlite3_stmt *queryStmt;
            auto len = sprintf(buffer, "SELECT * FROM %s ORDER BY rowid;", ast->target_table->to_string().c_str());
            assert(sqlite3_prepare_v2(db,buffer,len,&queryStmt,nullptr) == SQLITE_OK);
            while (sqlite3_step(queryStmt) == SQLITE_ROW) {
                std::string insertSql = "INSERT INTO " + ast->target_table->to_string() + " VALUES (";
                auto masterRoot = ast->master;
                auto index = 0;
                while (masterRoot) {
                    auto type = masterRoot->left;
                    switch (type->type) {
                        case token_type::Int: {
                            auto num = sqlite3_column_int64(queryStmt, index);
                            insertSql.append(std::to_string(num));
                            break;
                        }
                        case token_type::text: {
                            auto text = sqlite3_column_text(queryStmt, index);
                            auto size = sqlite3_column_bytes(queryStmt, index);
                            insertSql.append("'");
                            insertSql.append(reinterpret_cast<const char *>(text), size);
                            insertSql.append("'");
                            break;
                        }
                        case token_type::Float: {
                            auto num = sqlite3_column_double(queryStmt, index);
                            auto size = sprintf(buffer,"%.64lf",num);
                            insertSql.append(std::string(buffer, size));
                            break;
                        }
                        default:
                            throw std::runtime_error("unsupported type in ImportDataFromSqlite()");
                    }
                    masterRoot = masterRoot->next;
                    index++;
                    if (masterRoot)
                        insertSql.append(",");
                }
                insertSql.append(");");
                assert(IEDB_execute_sql_without_reader(insertSql.c_str()) == status_ok);
            }
        }
    }

    void ExportDataToSqlite(sqlite3 *db) {
        static char buffer[4096];
        static int bufferLen;
        const auto &manager = dbManager::get_instance();
        bufferLen = 0;
        for (auto &table: manager.map) {
            //在sqlite中创建对应表
            const auto &name = table.first;
            const auto &value = table.second;
            bufferLen = sprintf(buffer, "CREATE TABLE %s (", name.c_str());
            auto colCount = value->_table->get_col_count();
            assert(colCount > 0);
            std::vector<const col_def *> cols(colCount);
            for (auto i = 0; i < colCount; i++) {
                cols[i] = value->_table->get_col_by_index(i);
                auto type = cols[i]->type;
                switch (type) {
                    case column_type::Int: {
                        bufferLen += sprintf(buffer + bufferLen, "%s int,\n", cols[i]->name);
                        break;
                    }
                    case column_type::text: {
                        bufferLen += sprintf(buffer + bufferLen, "%s text,\n", cols[i]->name);
                        break;
                    }
                    case column_type::Float: {
                        bufferLen += sprintf(buffer + bufferLen, "%s real,\n", cols[i]->name);
                        break;
                    }
                    default:
                        throw std::runtime_error("unsupported type in ExportDataToSqlite()");
                }
            }
            bufferLen -= 2;
            bufferLen += sprintf(buffer + bufferLen, ");");
            sqlite3_exec(db, buffer, nullptr, nullptr, nullptr);
            //开始准备插入数据
            bufferLen = sprintf(buffer, "INSERT INTO  %s VALUES (", name.c_str());
            for (auto i = 0; i < colCount; i++) {
                bufferLen += sprintf(buffer + bufferLen, "?,");
            }
            bufferLen -= 1;
            bufferLen += sprintf(buffer + bufferLen, ");");
            sqlite3_stmt *stmt;
            DBreader *reader;
            sqlite3_prepare_v2(db, buffer, bufferLen, &stmt, nullptr);
            bufferLen = sprintf(buffer, "SELECT * FROM %s;", name.c_str());
            assert(IEDB_execute_query_sql(buffer,&reader) == status_ok);
            auto count = 0;
            while (IEDB_reader_next(reader) == status_ok) {
                count++;
                for (auto i = 0; i < colCount; i++) {
                    auto type = cols[i]->type;
                    switch (type) {
                        case column_type::Int: {
                            auto num = IEDB_reader_get_column_int(reader, i);
                            sqlite3_bind_int64(stmt, i + 1, num);
                            break;
                        }
                        case column_type::text: {
                            auto string = IEDB_reader_get_column_string(reader, i);
                            auto bytes = IEDB_reader_get_column_bytes(reader, i);
                            sqlite3_bind_text(stmt, i + 1, string, bytes, nullptr);
                            break;
                        }
                        case column_type::Float: {
                            auto num = IEDB_reader_get_column_double(reader, i);
                            sqlite3_bind_double(stmt, i + 1, num);
                            break;
                        }
                        default:
                            throw std::runtime_error("unsupported type in ExportDataToSqlite()");
                    }
                }
                assert(sqlite3_step(stmt) == SQLITE_DONE);
                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);
            }
            sqlite3_finalize(stmt);
            IEDB_reader_free(reader);
        }
    }
}
