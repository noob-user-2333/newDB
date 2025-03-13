//
// Created by user on 25-1-17.
//
#include "table.h"
#include <cmath>
namespace iedb
{
    column_value& row::operator[](const int index)
    {
        return values.at(index);
    }

    column_type row::get_column_type(const column_value& data)
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

    // 注:所有情况下字符串单独处理，不会进入operation函数
    // 工具函数：处理两个 column_data_type 的运算
    template <typename Op>
    column_value apply_operation(const column_value& v1, const column_value& v2,
                                     Op op)
    {
        return std::visit([&op](auto&& lhs, auto&& rhs) -> column_value
        {
            using LType = std::decay_t<decltype(lhs)>;
            using RType = std::decay_t<decltype(rhs)>;
            if constexpr (std::is_same_v<LType, int64> && std::is_same_v<RType, int64>)
            {
                return op(lhs, rhs);
            }
            if constexpr ((std::is_arithmetic_v<LType> && std::is_arithmetic_v<RType>))
            {
                const auto left = static_cast<double>(lhs);
                const auto right = static_cast<double>(rhs);
                //如果是 std::modulus，使用 std::fmod
                if constexpr (std::is_same_v<Op, std::modulus<>>) {
                    return column_value{std::fmod(left, right)};
                }else
                {
                    return column_value(op(left,right));
                }
            }
            throw std::runtime_error("should not run here of apply_operation");
        }, v1, v2);
    }

    int64 apply_compare_operation(const column_value& v1, const column_value& v2)
    {
        return std::visit([](auto&& lhs, auto&& rhs) -> int64
        {
            using LType = std::decay_t<decltype(lhs)>;
            using RType = std::decay_t<decltype(rhs)>;
            if constexpr (std::is_same_v<LType, int64> && std::is_same_v<RType, int64>)
            {
                return std::minus<>{}(lhs, rhs);
            }
            if constexpr ((std::is_arithmetic_v<LType> && std::is_arithmetic_v<RType>)) {
                const auto left = static_cast<double>(lhs);
                const auto right = static_cast<double>(rhs);

                const double result = left - right;
                if (result > 0.0)
                    return 1;
                if (result < 0.0)
                    return -1;
                return 0;
            }
            throw std::runtime_error("should not run here of apply_compare_operation");
        }, v1, v2);
    }

    // 加法运算
    int row::column_data_add(const column_value& data1, const column_value& data2, column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        out_result = apply_operation(data1, data2, std::plus<>());
        return status_ok;
    }

    int row::column_data_minus(const column_value& data1, const column_value& data2,
                               column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        out_result = apply_operation(data1, data2, std::minus<>());
        return status_ok;
    }

    int row::column_data_mul(const column_value& data1, const column_value& data2, column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        out_result = apply_operation(data1, data2, std::multiplies<>());
        return status_ok;
    }

    int row::column_data_div(const column_value& data1, const column_value& data2, column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        out_result = apply_operation(data1, data2, std::divides<>());
        return status_ok;
    }

    int row::column_data_mod(const column_value& data1, const column_value& data2, column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        out_result = apply_operation(data1, data2, std::modulus<>());
        return status_ok;
    }

    int row::column_data_more(const column_value& data1, const column_value& data2,
                              column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        auto value = apply_compare_operation(data1, data2);
        out_result = value > 0 ? 1 : 0;
        return status_ok;
    }

    int row::column_data_less(const column_value& data1, const column_value& data2,
                              column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        auto value = apply_compare_operation(data1, data2);
        out_result = value < 0 ? 1 : 0;
        return status_ok;
    }

    int row::column_data_more_equal(const column_value& data1, const column_value& data2,
                                    column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        auto value = apply_compare_operation(data1, data2);
        out_result = value >= 0 ? 1 : 0;
        return status_ok;
    }

    int row::column_data_less_equal(const column_value& data1, const column_value& data2,
                                    column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
            return status_invalid_argument;
        const auto value = apply_compare_operation(data1, data2);
        out_result = value <= 0 ? 1 : 0;
        return status_ok;
    }

    int row::column_data_equal(const column_value& data1, const column_value& data2,
                               column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
        {
            if (type1 == type2)
            {
                auto& str1 = std::get<std::string>(data1);
                auto& str2 = std::get<std::string>(data2);
                if (str1 == str2)
                    out_result = 1;
                else
                    out_result = 0;
                return status_ok;
            }
            return status_invalid_argument;
        }
        const auto value = apply_compare_operation(data1, data2);
        out_result = value == 0 ? 1 : 0;
        return status_ok;
    }

    int row::column_data_not_equal(const column_value& data1, const column_value& data2,
                                   column_value& out_result)
    {
        const auto type1 = get_column_type(data1);
        const auto type2 = get_column_type(data2);
        if (type1 == column_type::text || type2 == column_type::text)
        {
            if (type1 == type2)
            {
                auto& str1 = std::get<std::string>(data1);
                auto& str2 = std::get<std::string>(data2);
                if (str1 != str2)
                    out_result = 1;
                else
                    out_result = 0;
                return status_ok;
            }
            return status_invalid_argument;
        }
        const auto value = apply_compare_operation(data1, data2);
        out_result = value != 0 ? 1 : 0;
        return status_ok;
    }
}
