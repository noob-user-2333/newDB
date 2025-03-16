%token_prefix TK_
%token SPACE STRING HEX NUMBER_INT NUMBER_FLOAT NAME COMMA SEMI DOT PARENTHESIS_LEFT
PARENTHESIS_RIGHT BRACKET_LEFT BRACKET_RIGHT BRACE_LEFT BRACE_RIGHT PLUS MINUS STAR SLASH MORE_EQUAL
LESS_EQUAL NOT_EQUAL MORE LESS BANG EQUAL BIT_OR BIT_AND PERCENT NOT AND
OR SELECT CREATE UPDATE INSERT DELETE DROP FROM WHERE ORDER
GROUP BY TABLE INT FLOAT TEXT INTO VALUES .


%token_type { token* }
%extra_argument {AST*ast }
%parse_failure {
     fprintf(stdout,"Parser failed\n");
     ast->type = token_type::error;
}
%syntax_error{
    fprintf(stdout,"syntax error happened\n");
    ast->type = token_type::error;
}
%include{
    #include "AST.h"

    using namespace iedb;
}//end include
//查询语句
cmd ::= SELECT colnames(A) FROM NAME(B) where_statement(C)  group_statement(E) order_statement(D) SEMI. {
    ast->type = token_type::select;
    ast->target_table = B;

    ast->master = A;
    ast->where = C;
    ast->order = D;
    ast->group = E;
    }
where_statement(A) ::= /* empty */. {A=nullptr; }
where_statement(A) ::= WHERE exprs(B). {A=B;}
order_statement(A) ::= /* empty */.{A=nullptr;}
order_statement(A) ::= ORDER BY exprs(B).{A=B;}
group_statement(A) ::= /* empty */.{A=nullptr;}
group_statement(A) ::= GROUP BY exprs(B).{A=B;}

colnames(A) ::= STAR(B). {A=B;}
colnames(A) ::= exprs(B). {A=B;}
exprs(A) ::= exprs(B) COMMA or_expr(C). {
    token* temp = B;
    while(temp->next!= nullptr) temp = temp->next;
    temp->next = C;
    A=B;}
exprs(A) ::= or_expr(B). {A=B;}

item(A) ::= NAME(B).   {A=B;}
item(A) ::= HEX(B).     {A=B;}
item(A) ::= NUMBER_INT(B).     {A=B;}
item(A) ::= NUMBER_FLOAT(B).     {A=B;}


op1(A) ::= PLUS(B) .     {A=B;}
op1(A) ::= MINUS(B).     {A=B;}
op2(A) ::= STAR(B).     {A=B;}
op2(A) ::= SLASH(B).     {A=B;}
op2(A) ::= PERCENT(B).     {A=B;}
op3(A) ::= MORE(B).     {A=B;}
op3(A) ::= LESS(B).     {A=B;}
op3(A) ::= EQUAL(B).     {A=B;}
op3(A) ::= NOT_EQUAL(B).     {A=B;}
op3(A) ::= MORE_EQUAL(B).     {A=B;}
op3(A) ::= LESS_EQUAL(B).     {A=B;}


or_expr(A) ::= or_expr(B) OR(D) and_expr(C). {D->left = B; D->right = C;A=D;}
or_expr(A) ::= and_expr(B). {A=B;}

and_expr(A) ::= and_expr(B) AND(D) compare_expr(C). {D->left = B; D->right = C;A=D;}
and_expr(A) ::= compare_expr(B). {A=B;}

compare_expr(A) ::= compare_expr(B) op3(D) compute_expr1(C).{D->left = B; D->right = C;A=D;}
compare_expr(A) ::= compute_expr1(B). {A=B;}

compute_expr1(A) ::= compute_expr1(B) op1(D) compute_expr2(C).{D->left = B; D->right = C;A=D;}
compute_expr1(A) ::= compute_expr2(B).{A=B;}

compute_expr2(A) ::= compute_expr2(B) op2(D) factor(C).{D->left = B; D->right = C;A=D;}
compute_expr2(A) ::= factor(B).{A=B;}

factor(A) ::= item(B).{A=B;}
factor(A) ::= PARENTHESIS_LEFT or_expr(B) PARENTHESIS_RIGHT. {A=B;}




//插入语句
cmd ::= INSERT INTO NAME(A) VALUES PARENTHESIS_LEFT data_values(B) PARENTHESIS_RIGHT SEMI . {
    ast->type = token_type::insert;
    ast->target_table = A;
    ast->master = B;
}
data_values(A) ::= data_values(B) COMMA value(C).{
    token* temp = B;
    while(temp->next!= nullptr) temp = temp->next;
    temp->next = C;
    A=B;}
data_values(A) ::= value(B).{A=B;}

value(A) ::= STRING(B).{A=B;}
value(A) ::= NUMBER_INT(B).{A=B;}
value(A) ::= HEX(B).{A=B;}
value(A) ::= NUMBER_FLOAT(B).{A=B;}

//建表语句
cmd ::= CREATE TABLE NAME(A) PARENTHESIS_LEFT colsdef(B) PARENTHESIS_RIGHT SEMI.{
    ast->type = token_type::create;
    ast->target_table = A;
    ast->master = B;
}
colsdef(A) ::= colsdef(B) COMMA coldef(C).{
    token* temp = B;
    while(temp->next!= nullptr) temp = temp->next;
    temp->next = C;
    A=B;}
colsdef(A) ::= coldef(B).{A=B;}
coldef(A) ::= NAME(B) type(C).{B->left = C; A=B;}

type(A) ::= INT(B).{A=B;}
type(A) ::= FLOAT(B).{A=B;}
type(A) ::= TEXT(B).{A=B;}

//删除语句
cmd ::= DELETE FROM NAME(A) where_statement(B) SEMI.{
    ast->type = token_type::Delete;
    ast->target_table = A;
    ast->where = B;
}
//删表语句
cmd ::= DROP TABLE NAME(A) SEMI.{
    ast->type = token_type::drop;
    ast->target_table = A;
}