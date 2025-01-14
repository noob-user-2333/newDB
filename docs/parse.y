%token_prefix TK_
%token SPACE STRING HEX NUMBER_INT NUMBER_FLOAT NAME COMMA SEMI DOT PARENTHESIS_LEFT
PARENTHESIS_RIGHT BRACKET_LEFT BRACKET_RIGHT BRACE_LEFT BRACE_RIGHT PLUS MINUS STAR SLASH MORE_EQUAL
LESS_EQUAL NOT_EQUAL MORE LESS BANG EQUAL BIT_OR BIT_AND PERCENT NOT AND
OR SELECT CREATE UPDATE INSERT DELETE DROP FROM WHERE ORDER
GROUP BY TABLE INT FLOAT TEXT INTO VALUES .


%token_type { token* }
%extra_argument {parse_result *result}
%parse_failure {
     fprintf(stdout,"Parser failed\n");
     result->type = token_type::error;
}
%syntax_error{
    fprintf(stdout,"syntax error happened\n");
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
    result->group = E;
    }
where_statement(A) ::= /* empty */. {A=nullptr; }
where_statement(A) ::= WHERE exprs(B). {A=B;}
order_statement(A) ::= /* empty */.{A=nullptr;}
order_statement(A) ::= ORDER BY exprs(B).{A=B;}
group_statement(A) ::= /* empty */.{A=nullptr;}
group_statement(A) ::= GROUP BY exprs(B).{A=B;}
colnames(A) ::= STAR(B). {A=B;}
colnames(A) ::= exprs(B). {A=B;}
exprs(A) ::= exprs(B) COMMA expr(C). {
    token* temp = B;
    while(temp->brother!= nullptr) temp = temp->brother;
    temp->brother = C;
    A=B;}
exprs(A) ::= expr(B). {A=B;}

item(A) ::= NAME(B).   {A=B;}
item(A) ::= HEX(B).     {A=B;}
item(A) ::= NUMBER_INT(B).     {A=B;}
item(A) ::= NUMBER_FLOAT(B).     {A=B;}


op(A) ::= PLUS(B) .     {A=B;}
op(A) ::= MINUS(B).     {A=B;}
op(A) ::= STAR(B).     {A=B;}
op(A) ::= SLASH(B).     {A=B;}
op(A) ::= BIT_OR(B).     {A=B;}
op(A) ::= BIT_AND(B).     {A=B;}
op(A) ::= PERCENT(B).     {A=B;}
op(A) ::= MORE(B).     {A=B;}
op(A) ::= LESS(B).     {A=B;}
op(A) ::= EQUAL(B).     {A=B;}
op(A) ::= NOT_EQUAL(B).     {A=B;}
op(A) ::= MORE_EQUAL(B).     {A=B;}
op(A) ::= LESS_EQUAL(B).     {A=B;}
op(A) ::= AND(B).     {A=B;}
op(A) ::= OR(B).     {A=B;}

factor(A) ::= factor(B) op(C) item(D). {
    token* temp = B;
    while(temp->child) temp = temp->child;
    temp->child = C;
    C->child = D;
    A = B;}
factor(A) ::= item(B).   {A=B;}

factor(A) ::= PARENTHESIS_LEFT(B) factor(C) PARENTHESIS_RIGHT(D). {
    B->child = C;
    token * temp = C;
    while(temp->child) temp = temp->child;
    temp->child = D;
    A = B;}

factor(A) ::= factor(B) op(C) PARENTHESIS_LEFT(D) factor(E) PARENTHESIS_RIGHT(F). {
    token * temp = B;
    while(temp->child) temp = temp->child;
    temp->child = C;
    C->child = D;
    D->child = E;
    temp = E;
    while(temp->child) temp = temp->child;
    temp->child = F;
    A = B;}


expr(A) ::= factor(B).{A=B;}




//插入语句
cmd ::= INSERT INTO NAME(A) VALUES PARENTHESIS_LEFT data_values(B) PARENTHESIS_RIGHT SEMI . {
    result->type = token_type::insert;
    result->target = A;
    result->master = B;
}
data_values(A) ::= data_values(B) COMMA value(C).{
    token* temp = B;
    while(temp->brother!= nullptr) temp = temp->brother;
    temp->brother = C;
    A=B;}
data_values(A) ::= value(B).{A=B;}

value(A) ::= STRING(B).{A=B;}
value(A) ::= NUMBER_INT(B).{A=B;}
value(A) ::= HEX(B).{A=B;}
value(A) ::= NUMBER_FLOAT(B).{A=B;}

//建表语句
cmd ::= CREATE TABLE NAME(A) PARENTHESIS_LEFT colsdef(B) PARENTHESIS_RIGHT SEMI.{
    result->type = token_type::create;
    result->target = A;
    result->master = B;
}
colsdef(A) ::= colsdef(B) COMMA coldef(C).{
    token* temp = B;
    while(temp->brother!= nullptr) temp = temp->brother;
    temp->brother = C;
    A=B;}
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