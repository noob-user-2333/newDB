//
// Created by user on 25-3-9.
//

#ifndef VDBE_H
#define VDBE_H
#include "vdbe_op.h"
#include "vdbe_cursor.h"

namespace iedb
{
    union vdbe_value {
        int64 num;
        double decimal;
        void* ptr;
    };
    struct vdbe
    {
        class vdbe_stack
        {
        private:
            std::vector<vdbe_value> values;
            int sp;
        public:
            vdbe_stack():values(32),sp(0){}
            void push(const vdbe_value& val) { values.emplace_back(val); sp++; }
            void pop() { if (sp > 0) sp--;else throw std::runtime_error("current stack is empty"); }
            vdbe_value& top() { return values.at(sp - 1); }
        };

        std::unique_ptr<table> _table;
        std::unique_ptr<vdbe_cursor> cursor;
        std::array<uint64,4> r;//通用寄存器
        std::array<double,4> xmm;//浮点寄存器
        vdbe_stack stack;
        std::vector<column_value> row;
        std::vector<std::vector<column_value>> result;
        int pc;
        int execute_instruct(const std::vector<uint8>&ins);
        vdbe();
    };
    using vdbe_func_ptr = int (*)(vdbe&);
}


#endif //VDBE_H
