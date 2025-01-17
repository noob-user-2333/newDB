//
// Created by user on 25-1-17.
//
#include "table.h"

namespace iedb
{
    column_data_type& row::operator[](const int index)
    {
            if (index >= size)
                throw std::out_of_range("row index out of range");
            return values[index];
    }
    column_data_type& row::operator[](const int64 index)
    {
            if (index >= size)
                throw std::out_of_range("row index out of range");
            return values[index];
    }
    column_type row::get_column_type(column_data_type&data)
    {
            switch (data.index())
            {
            case 0:
                return column_type::Int;
            case 1:
                return column_type::Float;
            case 2:
                return column_type::text;
            default:
                throw std::runtime_error("unknown column type");
            }
    }


    using Variant = column_data_type;
    // 工具函数：处理两个 Variant 的运算
    template <typename Op>
    Variant apply_operation(const Variant& v1, const Variant& v2, Op op) {
        return std::visit([&op](auto&& lhs, auto&& rhs) -> Variant {
            using LType = std::decay_t<decltype(lhs)>;
            using RType = std::decay_t<decltype(rhs)>;

            // 禁止字符串运算
            if constexpr (std::is_same_v<LType, std::string> || std::is_same_v<RType, std::string>) {
                throw std::invalid_argument("Arithmetic operations are not supported for std::string.");
            } else if constexpr (std::is_same_v<LType,int64> && std::is_same_v<RType,int64>){
                return op(lhs,rhs);
            }
            else{
                return op(static_cast<double>(lhs), static_cast<double>(rhs));
            }
        }, v1, v2);
    }

    // 加法运算
    Variant operator+(const Variant& v1, const Variant& v2) {
        return apply_operation(v1, v2, std::plus<>());
    }

    // 相等运算符 ==
    bool operator==(const Variant& v1, const Variant& v2) {
        return std::visit([](auto&& lhs, auto&& rhs) -> bool {
            using LType = std::decay_t<decltype(lhs)>;
            using RType = std::decay_t<decltype(rhs)>;

            if constexpr (std::is_same_v<LType, std::string> && std::is_same_v<RType, std::string>) {
                return lhs == rhs; // 字符串直接比较
            } else {
                return static_cast<double>(lhs) == static_cast<double>(rhs);
            }
        }, v1, v2);
    }

    // 不等运算符 !=
    bool operator!=(const Variant& v1, const Variant& v2) {
        return !(v1 == v2);
    }
// 减法运算
Variant operator-(const Variant& v1, const Variant& v2) {
    return apply_operation(v1, v2, std::minus<>());
}

// 乘法运算
Variant operator*(const Variant& v1, const Variant& v2) {
    return apply_operation(v1, v2, std::multiplies<>());
}

// 除法运算
Variant operator/(const Variant& v1, const Variant& v2) {
    return apply_operation(v1, v2, std::divides<>());
}

// 取余运算（仅适用于 long）
Variant operator%(const Variant& v1, const Variant& v2) {
    return apply_operation(v1, v2, std::modulus<>());
}



bool operator<(const Variant& v1, const Variant& v2) {
    return std::visit([](auto&& lhs, auto&& rhs) {
        return static_cast<double>(lhs) < static_cast<double>(rhs);
    }, v1, v2);
}

bool operator>(const Variant& v1, const Variant& v2) {
    return std::visit([](auto&& lhs, auto&& rhs) {
        return static_cast<double>(lhs) > static_cast<double>(rhs);
    }, v1, v2);
}

bool operator<=(const Variant& v1, const Variant& v2) {
    return !(v1 > v2);
}

bool operator>=(const Variant& v1, const Variant& v2) {
    return !(v1 < v2);
}



}