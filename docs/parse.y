%token_prefix TK_
%token SPACE STRING HEX NUMBER_INT NUMBER_FLOAT NAME COMMA SEMI DOT PARENTHESIS_LEFT
PARENTHESIS_RIGHT BRACKET_LEFT BRACKET_RIGHT BRACE_LEFT BRACE_RIGHT PLUS MINUS STAR SLASH MORE_EQUAL
LESS_EQUAL NOT_EQUAL MORE LESS BANG EQUAL BIT_OR BIT_AND NOT AND
OR SELECT CREATE UPDATE INSERT DELETE DROP FROM WHERE ORDER
GROUP BY TABLE INT8 INT16 INT32 INT64 UINT8 UINT16 UINT32
UINT64 FLOAT INTO VALUES .
%token_type { token* }
%extra_argument {parse_result *result}
%parse_failure {
     fprintf(stderr,"Giving up.  Parser is hopelessly lost...\n");
     result->type = token_type::error;
}
%include{
    #include "parser.h"

    using namespace iedb;
}//end include
//查询语句
cmd ::= SELECT colnames(A) FROM NAME(B) where_statement(C) order_statement(D) group_statement(E) SEMI. {
    result->type = token_type::select;
    result->master = A;
    result->target = B;
    result->filter = C;
    result->order = D;
    result->group = E;}
where_statement(A) ::= WHERE exprs(B). {A=B;}
where_statement(A) ::= . {A=nullptr;}
order_statement(A) ::= ORDER BY exprs(B).{A=B;}
order_statement(A) ::= .{A=nullptr;}
group_statement(A) ::= GROUP BY exprs(B).{A=B;}
group_statement(A) ::= .{A=nullptr;}
colnames(A) ::= STAR(B). {A=B;}
colnames(A) ::= exprs(B). {A=B;}
exprs(A) ::= exprs(B) COMMA expr(C). {B->brother = C; A=B;}
exprs(A) ::= expr(B). {A=B;}

item(A) ::= NAME(B).   {A = B;}
item(A) ::= HEX(B).     {A=B;}
item(A) ::= NUMBER_INT(B).     {A=B;}
item(A) ::= NUMBER_FLOAT(B).     {A=B;}

op(A) ::= PLUS(B) .     {A=B;}
op(A) ::= MINUS(B).     {A=B;}
op(A) ::= STAR(B).     {A=B;}
op(A) ::= SLASH(B).     {A=B;}
op(A) ::= BIT_OR(B).     {A=B;}
op(A) ::= BIT_AND(B).     {A=B;}
op(A) ::= MORE(B).     {A=B;}
op(A) ::= LESS(B).     {A=B;}
op(A) ::= EQUAL(B).     {A=B;}
op(A) ::= MORE_EQUAL(B).     {A=B;}
op(A) ::= LESS_EQUAL(B).     {A=B;}

expr(A) ::= item(B).   {A=B;}
expr(A) ::= item(B) op(C) item(D). {B->child = C; C->child = D; A = B;}

//建表语句
cmd ::= CREATE TABLE NAME(A) BRACE_LEFT colsdef(B) BRACE_RIGHT SEMI.{
    result->type = token_type::create;
    result->target = A;
    result->master = B;
}
colsdef(A) ::= colsdef(B) COMMA coldef(C).{B->brother = C; A=B;}
colsdef(A) ::= coldef(B).{A=B;}
coldef(A) ::= NAME(B) type(C).{B->child = C; A=B;}

type(A) ::= INT(B).{A=B;}
type(A) ::= FLOAT(B).{A=B;}
type(A) ::= TEXT(B).{A=B;}

//删除语句
cmd ::= DELETE FROM NAME(A) where_statement(B) SEMI.{
    result->type = token_type::Delete;
    result->target = A;
    result->filter = B;
}
//删表语句
cmd ::= DROP TABLE NAME(A) SEMI.{
    result->type = token_type::drop;
    result->target = A;
}