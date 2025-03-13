//
// Created by user on 25-3-5.
//

#ifndef VDBEOP_H
#define VDBEOP_H
#include "utility.h"
namespace iedb
{
    //所有指令格式大致为 op dest src
    enum class vdbe_op:uint8_t
    {
        error,//不包含操作数
        call,//后接8字节函数地址
        push,//后跟1字节整数，表示将r[n]送入栈
        pop,//后跟1字节整数，表示将出栈数据送到r[n]
        //用于运算的指令为3字节
        //所有运算均为整数与整数 浮点数与浮点数的运算
        //op dest src
        //等价于 r[dest] = r[dest] op r[src]
        add,
        minus,
        mul,
        div,
        mod,
        And,
        Or,
        //后跟两个字节表示
        // r[dest] = (double)r[src]
        // r[dest] = (int64)r[src]
        num_to_float,
        float_to_num,
        cmp,//比较指令，暂不考虑字符串比较
        //对于load指令,后跟9个字节
        //第一个字节对应第几个寄存器，后8个字节对应数据
        move_imm_num,
        move_imm_decimal,
        move_imm_ptr,
        move_col, //用于装载列数据，后跟2字节整数，表示对应第几列
        //暂时不考虑删除语句
        //跳转语句 指令码后跟4字节整数表示跳转地址
        jmp,//无条件跳转
        jz,//r[0] = 0
        jnz,//r[0] != 0
        jl, // r[0] < 0
        jg, // r[0] > 0
        jle, // r[0] <= 0
        jge, // r[0] >= 0
    };
    // struct vdbe_op
    // {
    //     opcodes op;
    //     int p1;
    //     int p2;
    //     union
    //     {
    //         uint64 num;
    //         double decimal;
    //         void* ptr;
    //     }p3;
    //     vdbe_op(opcodes op, int p1, int p2,uint64 p3):op(op), p1(p1), p2(p2),p3()
    //     {
    //         this->p3.num = p3;
    //     }
    //     vdbe_op(opcodes op, int p1, int p2, double p3):op(op), p1(p1), p2(p2), p3()
    //     {
    //         this->p3.decimal = p3;
    //     }
    //     vdbe_op(opcodes op, int p1, int p2,void* p3):op(op), p1(p1), p2(p2), p3()
    //     {
    //         this->p3.ptr = p3;
    //     }
    // };




}



#endif //VDBEOP_H
