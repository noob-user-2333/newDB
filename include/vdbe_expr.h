//
// Created by user on 25-3-13.
//

#ifndef VDBE_EXPR_H
#define VDBE_EXPR_H
#include "table.h"
#include <xxhash.h>

namespace iedb {
    class vdbe_expr {
    private:
        struct expr_op {
            token_type type;
            column_value value;

            expr_op(token_type type, int64 value) : type(type), value(value) {
            }

            expr_op(token_type type, double value) : type(type), value(value) {
            }

            expr_op(token_type type, std::string value) : type(type), value(std::move(value)) {
            }
        };

        std::vector<expr_op> expr;
        std::stack<column_value> stack;
        static expr_op convert_token_to_expr_op(const table &_table, token *token_op);

        static void expr_tree_to_suffix(const table &_table, std::vector<expr_op> &expr, token *root);

        explicit vdbe_expr(std::vector<expr_op> &expr);
        void execute_ins(const expr_op& op,const std::vector<column_value>&row_data);
    public:
        class Hash {
        protected:
            XXH64_state_t* state;
            Hash();
        public:
            friend class vdbe_expr;
            ~Hash();
            void reset() const;
            [[nodiscard]] uint64 get_hash() const;
        };
        static std::unique_ptr<vdbe_expr> generate(const table &_table, token *root);
        static bool is_zero(const column_value&value);
        static std::unique_ptr<Hash> get_Hash();
        column_value run(const std::vector<column_value> &row_data) ;
        void update_hash(Hash&hash,const std::vector<column_value> &row_data);
    };
}
#endif //VDBE_EXPR_H
