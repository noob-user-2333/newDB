//
// Created by user on 25-3-6.
//

#ifndef VDBE_STMT_H
#define VDBE_STMT_H
#include "vdbe_cursor.h"
#include "vdbe.h"

namespace iedb
{
    class vdbe_stmt
    {
    private:
    //
    //
    //     std::unique_ptr<parse_result> result;
    //     std::vector<uint8> ins;
    //
    //     static void append_call(std::vector<uint8>&ins,vdbe_func_ptr func);
    //     static void append_new_col(std::vector<uint8>&ins,int64 name_len,void*name_start,int64 type,int64 element_count);
    //     static void append_push(std::vector<uint8>&ins,int r_index);
    //     static void append_load_imm(std::vector<uint8>& ins,int r_index,int64 num);
    //     static void append_load_imm(std::vector<uint8>& ins,int r_index,double decimal);
    //     static void append_load_imm(std::vector<uint8>& ins,int r_index,void * ptr);
    //
    //
    //     static std::unique_ptr<vdbe_stmt> prepare_create(std::unique_ptr<parse_result>& result);
    //     static std::unique_ptr<vdbe_stmt> prepare_insert(std::unique_ptr<parse_result>& result);
    //     static std::unique_ptr<vdbe_stmt> prepare_select(std::unique_ptr<parse_result>& result);
    //     vdbe_stmt(std::unique_ptr<parse_result>& result,std::vector<uint8>&ins);
    //     static std::unique_ptr<vdbe_stmt> prepare(std::unique_ptr<parse_result>& result);
    //     static token* convert_infix_to_suffix(token* root);
    //     static void convert_token_to_expr_op(std::vector<uint8>&ins,token* token_op);
    // public:
    //
    //     static std::unique_ptr<vdbe_stmt> prepare(const char*sql);
    //     static int execute(vdbe_stmt & stmt);
    //     static int execute_without_reader(const char*sql);




    };






}

#endif //VDBE_STMT_H
