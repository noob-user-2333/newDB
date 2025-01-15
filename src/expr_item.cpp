//
// Created by user on 25-1-15.
//
#include "expr.h"
#include <cmath>

namespace iedb
{
    expr::item expr::item::operator+(const item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, value_int + other.value_int};
            return {item_type::Float, static_cast<double>(value_int) + other.value_float};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Float, value_float + static_cast<double>(other.value_int)};
            return {item_type::Float, value_float + other.value_float};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator-(const item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, value_int - other.value_int};
            return {item_type::Float, static_cast<double>(value_int) - other.value_float};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Float, value_float - static_cast<double>(other.value_int)};
            return {item_type::Float, value_float - other.value_float};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator*(const item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, value_int * other.value_int};
            return {item_type::Float, static_cast<double>(value_int) * other.value_float};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Float, value_float * static_cast<double>(other.value_int)};
            return {item_type::Float, value_float * other.value_float};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator/(const item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, value_int / other.value_int};
            return {item_type::Float, static_cast<double>(value_int) / other.value_float};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Float, value_float / static_cast<double>(other.value_int)};
            return {item_type::Float, value_float / other.value_float};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator%(const item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, value_int % other.value_int};
            return {item_type::Float, std::fmod(static_cast<double>(value_int), other.value_float)};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Float, std::fmod(value_float, static_cast<double>(other.value_int))};
            return {item_type::Float, std::fmod(value_float, other.value_float)};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator>(const expr::item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_int > other.value_int)};
            return {item_type::Int, static_cast<int64>(static_cast<double>(value_int) > other.value_float)};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_float > static_cast<double>(other.value_int))};
            return {item_type::Int, static_cast<int64>(value_float > other.value_float)};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator>=(const expr::item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_int > other.value_int)};
            return {item_type::Int, static_cast<int64>(static_cast<double>(value_int) > other.value_float)};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_float > static_cast<double>(other.value_int))};
            return {item_type::Int, static_cast<int64>(value_float > other.value_float)};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator<(const expr::item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_int < other.value_int)};
            return {item_type::Int, static_cast<int64>(static_cast<double>(value_int) < other.value_float)};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_float < static_cast<double>(other.value_int))};
            return {item_type::Int, static_cast<int64>(value_float < other.value_float)};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator<=(const expr::item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_int <= other.value_int)};
            return {item_type::Int, static_cast<int64>(static_cast<double>(value_int) <= other.value_float)};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_float <= static_cast<double>(other.value_int))};
            return {item_type::Int, static_cast<int64>(value_float <= other.value_float)};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator==(const expr::item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_int == other.value_int)};
            return {item_type::Int, static_cast<int64>(static_cast<double>(value_int) == other.value_float)};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_float == static_cast<double>(other.value_int))};
            return {item_type::Int, static_cast<int64>(value_float == other.value_float)};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }

    expr::item expr::item::operator!=(const expr::item& other) const
    {
        if (other.type == item_type::String || type == item_type::String)
        {
            printf("请勿使用字符串类型数据进行不支持的运算操作\n");
            return {item_type::error, static_cast<int64>(status_invalid_sql)};
        }
        switch (type)
        {
        case item_type::Int:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_int == other.value_int)};
            return {item_type::Int, static_cast<int64>(static_cast<double>(value_int) == other.value_float)};
        case item_type::Float:
            if (other.type == item_type::Int)
                return {item_type::Int, static_cast<int64>(value_float == static_cast<double>(other.value_int))};
            return {item_type::Int, static_cast<int64>(value_float == other.value_float)};
        default:
            throw std::runtime_error("unsupported type for the operator");
        }
    }
}
