%token_prefix TK_
%token SPACE STRING HEX NUMBER NAME COMMA SEMI DOT PARENTHESIS_LEFT PARENTHESIS_RIGHT
BRACKET_LEFT BRACKET_RIGHT BRACE_LEFT BRACE_RIGHT PLUS MINUS STAR SLASH MORE LESS
BANG EQUAL BIT_OR BIT_AND NOT AND OR SELECT CREATE UPDATE
DELETE FROM WHERE ORDER GROUP BY
%token_type {token*}

%include{
    #include <token.h>


}//end include
//查询语句
cmd ::= SELECT exprs FROM NAME where_statement order_statement group_statement SEMI.
where_statement(A) ::= WHERE(B) exprs(C). {B->child = C;A=B;}
where_statement ::= .
order_statement(A) ::= ORDER(B) BY exprs(C).{B->child = C;A=B;}
order_statement ::= .
group_statement(A) ::= GROUP(B) BY exprs(C).{B->child = C;A=B;}
group_statement ::= .
exprs(A) ::= exprs(B) COMMA expr(C). {B->brother = C; A=B;}
exprs(A) ::= expr(B). {A=B;}

item(A) ::= NAME(B).   {A = B;}
item(A) ::= HEX(B).     {A=B;}
item(A) ::= NUMBER_INT(B).     {A=B;}
item(A) ::= NUMBER_FLOAT(B).     {A=B;}

op(A) ::= ADD(B) .     {A=B;}
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

//插入语句
cmd ::= INSERT INTO NAME VALUES PARENTHESIS_LEFT values PARENTHESIS_RIGHT.
values ::= values COMMA value.
values ::= value.
value ::= HEX.
value ::= NUMBER_INT.
value ::= NUMBER_FLOAT.
value ::= STRING.
//建表语句


//删除语句
cmd ::= DELETE FROM NAME where_statement.
//删表语句
cmd ::= DROP TABLE NAME.