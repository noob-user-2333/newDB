//
// Created by user on 25-3-6.
//
#include "vdbe_stmt.h"
#include <cmath>
#include <sys/stat.h>

namespace iedb
{
    vdbe::vdbe():pc(0)
    {

    }


//
//    int vdbe::execute_instruct(const std::vector<uint8>& ins)
//    {
//        auto type = static_cast<vdbe_op>(ins[pc]);
//        pc++;
//        switch (type)
//        {
//        case vdbe_op::error:
//            return status_error;
//        case vdbe_op::call:
//            {
//                auto func = (vdbe_func_ptr)(&ins[pc]);
////                r[0].num = func(*this);
//                pc += sizeof(void*);
//                break;
//            }
//        case vdbe_op::push:
//            {
//                auto index = ins[pc];
//                pc++;
//                stack.push(r.at(index));
//                break;
//            }
//        case vdbe_op::pop:
//            {
//                auto index = ins[pc];
//                pc++;
//                r.at(index) = stack.top();
//                stack.pop();
//                break;
//            }
//        case vdbe_op::add:
//            {
//                r[0] = apply_operation(r[0],r[1],std::plus<>());
//                break;
//            }
//        case vdbe_op::minus:
//            {
//                r[0] = apply_operation(r[0],r[1],std::minus<>());
//                break;
//            }
//        case vdbe_op::mul:
//            {
//                r[0] = apply_operation(r[0],r[1],std::multiplies<>());
//                break;
//            }
//        case vdbe_op::div:
//            {
//                r[0] = apply_operation(r[0],r[1],std::divides<>());
//                break;
//            }
//        case vdbe_op::mod:
//            {
//                r[0] = apply_operation(r[0],r[1],std::modulus<>());
//                break;
//            }
//        case vdbe_op::And:
//            {
//                auto value = is_zero(r[0]);
//                auto value2 = is_zero(r[1]);
//                r[0] = static_cast<int64>((value && value2) == true);
//                break;
//            }
//        case vdbe_op::Or:
//            {
//                auto value = is_zero(r[0]);
//                auto value2 = is_zero(r[1]);
//                r[0] = static_cast<int64>((value || value2) == true);
//                break;
//            }
//        case vdbe_op::cmp:
//            {
//                auto result = apply_compare_operation(r[0],r[1]);
//                r[0] = result;
//                break;
//            } //比较指令，暂不考虑字符串比较
//        case vdbe_op::load_imm_num:
//            {
//                const auto index = ins[pc];
//                pc++;
//                r[index] = *reinterpret_cast<const int64*>(&ins[pc]);
//                pc += sizeof(int64);
//                break;
//            }
//        case vdbe_op::load_imm_decimal:
//            {
//                const auto index = ins[pc];
//                pc++;
//                r[index] = *reinterpret_cast<const double*>(&ins[pc]);
//                pc += sizeof(double);
//                break;
//            }
//        case vdbe_op::load_imm_ptr:
//            {
//                const auto index = ins[pc];
//                pc++;
//                const auto value = *reinterpret_cast<const uint64*>(&ins[pc]);
//                r[index] = reinterpret_cast<void*>(value);
//                pc += sizeof(void*);
//                break;
//            }
//            //TODO:暂不考虑装载列数据
//        case vdbe_op::load_col: { break; } //用于装载列数据，后跟2字节整数，表示对应第几列
//        case vdbe_op::store_int:
//            {
//                auto data_start = ins.data() + pc;
//                auto value = *reinterpret_cast<const uint64*>(data_start);
//                row.emplace_back(value);
//                pc+=sizeof(uint64);
//                break;
//            }
//        case vdbe_op::store_double:
//            {
//                auto data_start = ins.data() + pc;
//                auto value = *reinterpret_cast<const double*>(data_start);
//                row.emplace_back(value);
//                pc += sizeof(double);
//                break;
//            }
//        case vdbe_op::store_string:
//            {
//                auto data_start = ins.data() + pc;
//                auto len = *reinterpret_cast<const int*>(data_start);
//                auto str_start = reinterpret_cast<const char*>(data_start + sizeof(len));
//                row.emplace_back(std::string(str_start,len));
//                pc += sizeof(int) + sizeof(void*);
//                break;
//            }
//        case vdbe_op::jmp:
//            {
//                auto offset = *reinterpret_cast<const int*>(&ins[pc]);
//                pc += 4;
//                pc += offset;
//                break;
//            } //无条件跳转
//        case vdbe_op::jz:
//            {
//                auto value = std::get<int64>(r[0]);
//                auto offset = *reinterpret_cast<const int*>(&ins[pc ]);
//                pc += 4;
//                if (value == 0)
//                    pc += offset;
//                break;
//            } //r[0] = 0
//        case vdbe_op::jnz:
//            {
//                auto value = std::get<int64>(r[0]);
//                auto offset = *reinterpret_cast<const int*>(&ins[pc]);
//                pc += 4;
//                if (value != 0)
//                    pc += offset;
//                break;
//            } //r[0] != 0
//        case vdbe_op::jl:
//            {
//                auto value = std::get<int64>(r[0]);
//                auto offset = *reinterpret_cast<const int*>(&ins[pc]);
//                pc += 4;
//                if (value < 0)
//                    pc += offset;
//                break;
//            } // r[0] < 0
//        case vdbe_op::jg:
//            {
//                auto value = std::get<int64>(r[0]);
//                auto offset = *reinterpret_cast<const int*>(&ins[pc]);
//                pc += 4;
//                if (value > 0)
//                    pc += offset;
//                break;
//            } // r[0] > 0
//        case vdbe_op::jle:
//            {
//                auto value = std::get<int64>(r[0]);
//                auto offset = *reinterpret_cast<const int*>(&ins[pc ]);
//                pc += 4;
//                if (value <= 0)
//                    pc += offset;
//                break;
//            } // r[0] <= 0
//        case vdbe_op::jge:
//            {
//                auto value = std::get<int64>(r[0]);
//                auto offset = *reinterpret_cast<const int*>(&ins[pc]);
//                pc += 4;
//                if (value >= 0)
//                    pc += offset;
//                break;
//            } // r[0] >= 0
//        default:
//            throw std::runtime_error("unknown vdbe_op for vdbe");
//        }
//        return status_ok;
//    }
}
