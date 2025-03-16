//
// Created by user on 25-3-9.
//

#ifndef VDBE_CALL_H
#define VDBE_CALL_H
#include "../include/vdbe.h"

namespace iedb
{
    class vdbe_call
    {
    public:
        static int begin_create_table(vdbe&_vdbe);
        static int add_new_column(vdbe&_vdbe);
        static int create_table(vdbe& _vdbe);
        static int insert(vdbe& _vdbe);
        static int open(vdbe& _vdbe);
        static int next(vdbe& _vdbe);
        static int store_row(vdbe& _vdbe);
        static int load_row(vdbe& _vdbe);
    };
}
#endif //VDBE_CALL_H
