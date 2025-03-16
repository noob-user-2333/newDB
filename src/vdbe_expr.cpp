//
// Created by user on 25-3-13.
//
#include "vdbe_expr.h"
#include <cmath>

namespace iedb {
    vdbe_expr::Hash::Hash() {
        state = XXH64_createState();
        XXH64_reset(state,0);
    }
    vdbe_expr::Hash::~Hash() {
        XXH64_freeState(state);
    }
    void vdbe_expr::Hash::reset() const {
        XXH64_reset(state,0);
    }

    uint64 vdbe_expr::Hash::get_hash() const {
        return XXH64_digest(state);
    }
    std::unique_ptr<vdbe_expr::Hash> vdbe_expr::get_Hash() {
        return std::unique_ptr<vdbe_expr::Hash>(new Hash());
    }



    // 注:所有情况下字符串单独处理，不会进入operation函数
    // 工具函数：处理两个 column_data_type 的运算
    template<typename Op>
    static column_value apply_operation(const column_value &v1, const column_value &v2,
                                        Op op) {
        return std::visit([&op](auto &&lhs, auto &&rhs) -> column_value {
            using LType = std::decay_t<decltype(lhs)>;
            using RType = std::decay_t<decltype(rhs)>;
            if constexpr (std::is_same_v<LType, int64> && std::is_same_v<RType, int64>) {
                return op(lhs, rhs);
            }
            if constexpr ((std::is_arithmetic_v<LType> && std::is_arithmetic_v<RType>)) {
                const auto left = static_cast<double>(lhs);
                const auto right = static_cast<double>(rhs);
                //如果是 std::modulus，使用 std::fmod
                if constexpr (std::is_same_v<Op, std::modulus<> >) {
                    return column_value{std::fmod(left, right)};
                } else {
                    return column_value(op(left, right));
                }
            }
            throw std::runtime_error("should not run here of apply_operation");
        }, v1, v2);
    }

    static int64 apply_compare_operation(const column_value &v1, const column_value &v2) {
        return std::visit([](auto &&lhs, auto &&rhs) -> int64 {
            using LType = std::decay_t<decltype(lhs)>;
            using RType = std::decay_t<decltype(rhs)>;
            if constexpr (std::is_same_v<LType, int64> && std::is_same_v<RType, int64>) {
                return std::minus<>{}(static_cast<int64>(lhs),static_cast<int64>(rhs));
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
    bool vdbe_expr::is_zero(const column_value& var) {
        return std::visit([](const auto& value) -> bool {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>) {
                return value == 0;
            } else if constexpr (std::is_same_v<T, std::string>) {
                return value.empty();
            }
            throw std::runtime_error("should not run to here"); // 理论上不会执行到这里
        }, var);
    }
    vdbe_expr::expr_op vdbe_expr::convert_token_to_expr_op(const table &_table, token *token_op) {
        switch (token_op->type) {
            case token_type::name: {
                auto index = _table.get_col_index_by_name(token_op->to_string());
                if (index < 0) {
                    char buff[1024];
                    sprintf(buff,"can not found column name of %s from %s\n",token_op->to_string().c_str(),_table.get_name().c_str());
                    throw std::runtime_error(buff);
                }
                return {token_type::name, static_cast<int64>(index)};
            }
            case token_type::string: {
                return {token_type::string, token_op->to_string()};
            }
            case token_type::number_int: {
                char *end;
                auto num = std::strtol(token_op->sql + token_op->offset, &end, 10);
                assert(end == token_op->sql + token_op->offset + token_op->len);
                return {token_type::number_int, static_cast<int64>(num)};
            }
            case token_type::number_float: {
                char *end;
                auto num = std::strtod(token_op->sql + token_op->offset, &end);
                assert(end == token_op->sql + token_op->offset + token_op->len);
                return {token_type::number_float, num};
            }
            case token_type::hex: {
                char *end;
                auto num = std::strtol(token_op->sql + token_op->offset + 2, &end, 16);
                assert(end == token_op->sql + token_op->offset + token_op->len);
                return {token_type::number_int, static_cast<int64>(num)};
            }
            case token_type::plus:
            case token_type::minus:
            case token_type::star:
            case token_type::slash:
            case token_type::percent:
            case token_type::less:
            case token_type::less_equal:
            case token_type::more:
            case token_type::more_equal:
            case token_type::equal:
            case token_type::not_equal:
            case token_type::And:
            case token_type::Or: {
                return {token_op->type, 0L};
            }
            default:
                throw std::runtime_error("unsupported token type");
        }
    }

    void vdbe_expr::execute_ins(const expr_op &op,const std::vector<column_value> & row_data) {
        auto type = op.type;
        switch (type) {
            case token_type::string:
            case token_type::hex:
            case token_type::number_int:
            case token_type::number_float: {
                // 直接入操作数栈
                stack.emplace(op.value);
                break;
            }
            case token_type::name: {
                auto index = std::get<int64_t>(op.value);
                stack.emplace(row_data.at(index));
                break;
            }
            case token_type::plus: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                stack.emplace(apply_operation(v1, v2,std::plus<>()));
                break;
            }
            case token_type::minus: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                stack.emplace(apply_operation(v1, v2,std::plus<>()));
                break;
            }
            case token_type::star: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                stack.emplace(apply_operation(v1, v2,std::multiplies<>()));
                break;
            }
            case token_type::slash: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                stack.emplace(apply_operation(v1, v2,std::divides<>()));
                break;
            }
            case token_type::percent: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                stack.emplace(apply_operation(v1, v2,std::modulus<>()));
                break;
            }
            case token_type::more_equal: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                auto result = apply_compare_operation(v1,v2);
                stack.emplace(result >= 0);
                break;
            }
            case token_type::less_equal: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                auto result = apply_compare_operation(v1,v2);
                stack.emplace(result <= 0);
                break;
            }
            case token_type::not_equal: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                auto result = apply_compare_operation(v1,v2);
                stack.emplace(result != 0);
                break;
            }
            case token_type::more: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                auto result = apply_compare_operation(v1,v2);
                stack.emplace(result > 0);
                break;
            }
            case token_type::less: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                auto result = apply_compare_operation(v1,v2);
                stack.emplace(result < 0);
                break;
            }
            case token_type::equal: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                auto result = apply_compare_operation(v1,v2);
                stack.emplace(result == 0);
                break;
            }
            case token_type::Not: {
                auto v = stack.top();
                stack.pop();
                auto b = is_zero(v);
                stack.emplace(b == false);
                break;
            }
            case token_type::And: {
                auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                auto result = (!is_zero(v1)) && (!is_zero(v2));
                stack.emplace(result);
                break;
            }
            case token_type::Or: {
                 auto v2 = stack.top();
                stack.pop();
                auto v1 = stack.top();
                stack.pop();
                auto result = (!is_zero(v1)) || (!is_zero(v2));
                stack.emplace(result);
                break;
            }
            default:
                throw std::runtime_error("unsupported token type");
        }
    }

    void vdbe_expr::expr_tree_to_suffix(const table &_table, std::vector<expr_op> &expr, token *root) {
        if (root->left)
            expr_tree_to_suffix(_table, expr, root->left);
        if (root->right)
            expr_tree_to_suffix(_table, expr, root->right);
        expr.push_back(convert_token_to_expr_op(_table, root));
    }

    vdbe_expr::vdbe_expr(std::vector<expr_op> &expr): expr(std::move(expr)) {
    }

    std::unique_ptr<vdbe_expr> vdbe_expr::generate(const table &_table, token *root) {
        std::vector<expr_op> expr;
        expr_tree_to_suffix(_table, expr, root);
        return std::unique_ptr<vdbe_expr>(new vdbe_expr(expr));
    }
    column_value vdbe_expr::run(const std::vector<column_value> &row_data) {
        assert(stack.empty());
        for (const auto&op : expr) {
            execute_ins(op,row_data);
        }
        auto v = stack.top();
        stack.pop();
        assert(stack.empty());
        return std::move(v);
    }
    void vdbe_expr::update_hash(Hash&hash,const std::vector<column_value> &row_data) {
        auto v = run(row_data);
        switch (v.index()) {
            case 0: {
                //uint64
                auto& value = std::get<int64>(v);
                XXH64_update(hash.state,&value,sizeof(value));
                return;
            }
                case 1: {
                //double
                auto& value = std::get<double>(v);
                XXH64_update(hash.state,&value,sizeof(value));
                return;
                }
            case 2: {
                //string
                const auto& value = std::get<std::string>(v);
                XXH64_update(hash.state,value.c_str(),value.size());
                return;
            }
            default:
                throw std::runtime_error("should not happened");
        }
    }

}
