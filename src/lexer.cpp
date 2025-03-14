/* Generated by re2c 3.1 on Fri Jan 10 15:42:52 2025 */
#line 1 "lexer.re"
#include "token.h"
namespace iedb {
#define re2c_token_process(x) { type = x; \
len = static_cast<uint32>(YYCURSOR - start); \
return type;}
    static token_type lexer_token(const char * start,const char *end,int& len){
         const char * YYCURSOR = start;
         const char * YYMARKER = YYCURSOR;
         const char * YYLIMIT = end;
         token_type type = token_type::error;

         
#line 16 "../src/lexer.cpp"
{
	char yych;
	yych = *YYCURSOR;
	switch (yych) {
		case '\t':
		case '\n':
		case '\r':
		case ' ': goto yy2;
		case '!': goto yy4;
		case '%': goto yy6;
		case '&': goto yy7;
		case '\'': goto yy8;
		case '(': goto yy9;
		case ')': goto yy10;
		case '*': goto yy11;
		case '+': goto yy12;
		case ',': goto yy13;
		case '-': goto yy14;
		case '.': goto yy15;
		case '/': goto yy16;
		case '0': goto yy17;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': goto yy19;
		case ';': goto yy21;
		case '<': goto yy22;
		case '=': goto yy24;
		case '>': goto yy25;
		case 'A':
		case 'a': goto yy27;
		case 'B':
		case 'b': goto yy29;
		case 'C':
		case 'c': goto yy30;
		case 'D':
		case 'd': goto yy31;
		case 'E':
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'P':
		case 'Q':
		case 'R':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'e':
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'p':
		case 'q':
		case 'r':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		case 'F':
		case 'f': goto yy34;
		case 'G':
		case 'g': goto yy35;
		case 'I':
		case 'i': goto yy36;
		case 'N':
		case 'n': goto yy37;
		case 'O':
		case 'o': goto yy38;
		case 'S':
		case 's': goto yy39;
		case 'T':
		case 't': goto yy40;
		case 'U':
		case 'u': goto yy41;
		case 'V':
		case 'v': goto yy42;
		case 'W':
		case 'w': goto yy43;
		case '[': goto yy44;
		case ']': goto yy45;
		case '{': goto yy46;
		case '|': goto yy47;
		case '}': goto yy48;
		default: goto yy1;
	}
yy1:
	++YYCURSOR;
#line 64 "lexer.re"
	{printf("something wrong start at %s\n",start);type = token_type::error;len = 1;return type;}
#line 115 "../src/lexer.cpp"
yy2:
	yych = *++YYCURSOR;
	switch (yych) {
		case '\t':
		case '\n':
		case '\r':
		case ' ': goto yy2;
		default: goto yy3;
	}
yy3:
#line 58 "lexer.re"
	{type = token_type::space; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 128 "../src/lexer.cpp"
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
		case '=': goto yy49;
		default: goto yy5;
	}
yy5:
#line 34 "lexer.re"
	{type = token_type::bang; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 138 "../src/lexer.cpp"
yy6:
	++YYCURSOR;
#line 38 "lexer.re"
	{re2c_token_process(token_type::percent)}
#line 143 "../src/lexer.cpp"
yy7:
	++YYCURSOR;
#line 37 "lexer.re"
	{type = token_type::bit_and; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 148 "../src/lexer.cpp"
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
		case '\'': goto yy50;
		default: goto yy8;
	}
yy9:
	++YYCURSOR;
#line 19 "lexer.re"
	{type = token_type::parenthesis_left; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 159 "../src/lexer.cpp"
yy10:
	++YYCURSOR;
#line 20 "lexer.re"
	{type = token_type::parenthesis_right; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 164 "../src/lexer.cpp"
yy11:
	++YYCURSOR;
#line 27 "lexer.re"
	{type = token_type::star; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 169 "../src/lexer.cpp"
yy12:
	++YYCURSOR;
#line 25 "lexer.re"
	{type = token_type::plus; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 174 "../src/lexer.cpp"
yy13:
	++YYCURSOR;
#line 16 "lexer.re"
	{type = token_type::comma; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 179 "../src/lexer.cpp"
yy14:
	++YYCURSOR;
#line 26 "lexer.re"
	{type = token_type::minus; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 184 "../src/lexer.cpp"
yy15:
	++YYCURSOR;
#line 18 "lexer.re"
	{type = token_type::dot; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 189 "../src/lexer.cpp"
yy16:
	++YYCURSOR;
#line 28 "lexer.re"
	{type = token_type::slash; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 194 "../src/lexer.cpp"
yy17:
	yych = *(YYMARKER = ++YYCURSOR);
	switch (yych) {
		case 'X':
		case 'x': goto yy53;
		default: goto yy20;
	}
yy18:
#line 60 "lexer.re"
	{type = token_type::number_int; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 205 "../src/lexer.cpp"
yy19:
	yych = *(YYMARKER = ++YYCURSOR);
yy20:
	switch (yych) {
		case '.': goto yy51;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': goto yy19;
		default: goto yy18;
	}
yy21:
	++YYCURSOR;
#line 17 "lexer.re"
	{type = token_type::semi; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 227 "../src/lexer.cpp"
yy22:
	yych = *++YYCURSOR;
	switch (yych) {
		case '=': goto yy54;
		default: goto yy23;
	}
yy23:
#line 31 "lexer.re"
	{type = token_type::less; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 237 "../src/lexer.cpp"
yy24:
	++YYCURSOR;
#line 35 "lexer.re"
	{type = token_type::equal; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 242 "../src/lexer.cpp"
yy25:
	yych = *++YYCURSOR;
	switch (yych) {
		case '=': goto yy55;
		default: goto yy26;
	}
yy26:
#line 30 "lexer.re"
	{type = token_type::more; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 252 "../src/lexer.cpp"
yy27:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'N':
		case 'n': goto yy56;
		default: goto yy33;
	}
yy28:
#line 62 "lexer.re"
	{type = token_type::name; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 263 "../src/lexer.cpp"
yy29:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'Y':
		case 'y': goto yy57;
		default: goto yy33;
	}
yy30:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'R':
		case 'r': goto yy59;
		default: goto yy33;
	}
yy31:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy60;
		default: goto yy33;
	}
yy32:
	yych = *++YYCURSOR;
yy33:
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy28;
	}
yy34:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'L':
		case 'l': goto yy61;
		case 'R':
		case 'r': goto yy62;
		default: goto yy33;
	}
yy35:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'R':
		case 'r': goto yy63;
		default: goto yy33;
	}
yy36:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'N':
		case 'n': goto yy64;
		default: goto yy33;
	}
yy37:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'O':
		case 'o': goto yy65;
		default: goto yy33;
	}
yy38:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'R':
		case 'r': goto yy66;
		default: goto yy33;
	}
yy39:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy68;
		default: goto yy33;
	}
yy40:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'A':
		case 'a': goto yy69;
		case 'E':
		case 'e': goto yy70;
		default: goto yy33;
	}
yy41:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'P':
		case 'p': goto yy71;
		default: goto yy33;
	}
yy42:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'A':
		case 'a': goto yy72;
		default: goto yy33;
	}
yy43:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'H':
		case 'h': goto yy73;
		default: goto yy33;
	}
yy44:
	++YYCURSOR;
#line 21 "lexer.re"
	{type = token_type::bracket_left; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 432 "../src/lexer.cpp"
yy45:
	++YYCURSOR;
#line 22 "lexer.re"
	{type = token_type::bracket_right; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 437 "../src/lexer.cpp"
yy46:
	++YYCURSOR;
#line 23 "lexer.re"
	{type = token_type::brace_left; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 442 "../src/lexer.cpp"
yy47:
	++YYCURSOR;
#line 36 "lexer.re"
	{type = token_type::bit_or; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 447 "../src/lexer.cpp"
yy48:
	++YYCURSOR;
#line 24 "lexer.re"
	{type = token_type::brace_right; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 452 "../src/lexer.cpp"
yy49:
	++YYCURSOR;
#line 33 "lexer.re"
	{type = token_type::not_equal; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 457 "../src/lexer.cpp"
yy50:
	++YYCURSOR;
#line 61 "lexer.re"
	{type = token_type::string; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 462 "../src/lexer.cpp"
yy51:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': goto yy74;
		default: goto yy52;
	}
yy52:
	YYCURSOR = YYMARKER;
	goto yy18;
yy53:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f': goto yy76;
		default: goto yy52;
	}
yy54:
	++YYCURSOR;
#line 32 "lexer.re"
	{type = token_type::less_equal; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 512 "../src/lexer.cpp"
yy55:
	++YYCURSOR;
#line 29 "lexer.re"
	{type = token_type::more_equal; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 517 "../src/lexer.cpp"
yy56:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'D':
		case 'd': goto yy78;
		default: goto yy33;
	}
yy57:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy58;
	}
yy58:
#line 50 "lexer.re"
	{type = token_type::by; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 596 "../src/lexer.cpp"
yy59:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy80;
		default: goto yy33;
	}
yy60:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'L':
		case 'l': goto yy81;
		default: goto yy33;
	}
yy61:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'O':
		case 'o': goto yy82;
		default: goto yy33;
	}
yy62:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'O':
		case 'o': goto yy83;
		default: goto yy33;
	}
yy63:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'O':
		case 'o': goto yy84;
		default: goto yy33;
	}
yy64:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'S':
		case 's': goto yy85;
		case 'T':
		case 't': goto yy86;
		default: goto yy33;
	}
yy65:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'T':
		case 't': goto yy88;
		default: goto yy33;
	}
yy66:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		case 'D':
		case 'd': goto yy90;
		default: goto yy67;
	}
yy67:
#line 41 "lexer.re"
	{type = token_type::Or; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 719 "../src/lexer.cpp"
yy68:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'L':
		case 'l': goto yy91;
		default: goto yy33;
	}
yy69:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'B':
		case 'b': goto yy92;
		default: goto yy33;
	}
yy70:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'X':
		case 'x': goto yy93;
		default: goto yy33;
	}
yy71:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'D':
		case 'd': goto yy94;
		default: goto yy33;
	}
yy72:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'L':
		case 'l': goto yy95;
		default: goto yy33;
	}
yy73:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy96;
		default: goto yy33;
	}
yy74:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': goto yy74;
		default: goto yy75;
	}
yy75:
#line 59 "lexer.re"
	{type = token_type::number_float; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 780 "../src/lexer.cpp"
yy76:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f': goto yy76;
		default: goto yy77;
	}
yy77:
#line 63 "lexer.re"
	{type = token_type::hex; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 811 "../src/lexer.cpp"
yy78:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy79;
	}
yy79:
#line 40 "lexer.re"
	{type = token_type::And; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 883 "../src/lexer.cpp"
yy80:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'A':
		case 'a': goto yy97;
		default: goto yy33;
	}
yy81:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy98;
		default: goto yy33;
	}
yy82:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'A':
		case 'a': goto yy99;
		default: goto yy33;
	}
yy83:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'M':
		case 'm': goto yy100;
		default: goto yy33;
	}
yy84:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'U':
		case 'u': goto yy102;
		default: goto yy33;
	}
yy85:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy103;
		default: goto yy33;
	}
yy86:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		case 'O':
		case 'o': goto yy104;
		default: goto yy87;
	}
yy87:
#line 52 "lexer.re"
	{re2c_token_process(token_type::Int)}
#line 997 "../src/lexer.cpp"
yy88:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy89;
	}
yy89:
#line 39 "lexer.re"
	{type = token_type::Not; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 1069 "../src/lexer.cpp"
yy90:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy106;
		default: goto yy33;
	}
yy91:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy107;
		default: goto yy33;
	}
yy92:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'L':
		case 'l': goto yy108;
		default: goto yy33;
	}
yy93:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'T':
		case 't': goto yy109;
		default: goto yy33;
	}
yy94:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'A':
		case 'a': goto yy111;
		default: goto yy33;
	}
yy95:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'U':
		case 'u': goto yy112;
		default: goto yy33;
	}
yy96:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'R':
		case 'r': goto yy113;
		default: goto yy33;
	}
yy97:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'T':
		case 't': goto yy114;
		default: goto yy33;
	}
yy98:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'T':
		case 't': goto yy115;
		default: goto yy33;
	}
yy99:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'T':
		case 't': goto yy116;
		default: goto yy33;
	}
yy100:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy101;
	}
yy101:
#line 46 "lexer.re"
	{type = token_type::from; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 1211 "../src/lexer.cpp"
yy102:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'P':
		case 'p': goto yy118;
		default: goto yy33;
	}
yy103:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'R':
		case 'r': goto yy120;
		default: goto yy33;
	}
yy104:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy105;
	}
yy105:
#line 57 "lexer.re"
	{re2c_token_process(token_type::into)}
#line 1297 "../src/lexer.cpp"
yy106:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'R':
		case 'r': goto yy121;
		default: goto yy33;
	}
yy107:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'C':
		case 'c': goto yy123;
		default: goto yy33;
	}
yy108:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy124;
		default: goto yy33;
	}
yy109:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy110;
	}
yy110:
#line 54 "lexer.re"
	{re2c_token_process(token_type::text)}
#line 1390 "../src/lexer.cpp"
yy111:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'T':
		case 't': goto yy126;
		default: goto yy33;
	}
yy112:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy127;
		default: goto yy33;
	}
yy113:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy128;
		default: goto yy33;
	}
yy114:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy130;
		default: goto yy33;
	}
yy115:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy132;
		default: goto yy33;
	}
yy116:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy117;
	}
yy117:
#line 53 "lexer.re"
	{re2c_token_process(token_type::Float)}
#line 1497 "../src/lexer.cpp"
yy118:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy119;
	}
yy119:
#line 49 "lexer.re"
	{type = token_type::group; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 1569 "../src/lexer.cpp"
yy120:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'T':
		case 't': goto yy134;
		default: goto yy33;
	}
yy121:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy122;
	}
yy122:
#line 48 "lexer.re"
	{type = token_type::order; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 1648 "../src/lexer.cpp"
yy123:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'T':
		case 't': goto yy136;
		default: goto yy33;
	}
yy124:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy125;
	}
yy125:
#line 55 "lexer.re"
	{re2c_token_process(token_type::table)}
#line 1727 "../src/lexer.cpp"
yy126:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'E':
		case 'e': goto yy138;
		default: goto yy33;
	}
yy127:
	yych = *++YYCURSOR;
	switch (yych) {
		case 'S':
		case 's': goto yy140;
		default: goto yy33;
	}
yy128:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy129;
	}
yy129:
#line 47 "lexer.re"
	{type = token_type::where; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 1813 "../src/lexer.cpp"
yy130:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy131;
	}
yy131:
#line 43 "lexer.re"
	{type = token_type::create; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 1885 "../src/lexer.cpp"
yy132:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy133;
	}
yy133:
#line 45 "lexer.re"
	{type = token_type::Delete; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 1957 "../src/lexer.cpp"
yy134:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy135;
	}
yy135:
#line 56 "lexer.re"
	{re2c_token_process(token_type::insert)}
#line 2029 "../src/lexer.cpp"
yy136:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy137;
	}
yy137:
#line 42 "lexer.re"
	{type = token_type::select; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 2101 "../src/lexer.cpp"
yy138:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy139;
	}
yy139:
#line 44 "lexer.re"
	{type = token_type::update; len = static_cast<uint32>(YYCURSOR - start);return type;}
#line 2173 "../src/lexer.cpp"
yy140:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy32;
		default: goto yy141;
	}
yy141:
#line 51 "lexer.re"
	{re2c_token_process(token_type::values)}
#line 2245 "../src/lexer.cpp"
}
#line 66 "lexer.re"

    }
	std::unique_ptr<std::vector<token>> token::lexer(const char *sql) {
		std::vector<token> tokens;
		auto sql_string = std::make_shared<std::string>(sql);
		auto len = strlen(sql);
		auto start = sql;
		auto end = start + len;
		while (start < end) {
			int token_len;
			//将字符串结束符号\0同样算作缓冲区中
			auto type = lexer_token(start, end + 1,  token_len);
			if (type == token_type::error)
				return nullptr;
			if(type != token_type::space)
			    tokens.emplace_back(type, static_cast<uint32>(start - sql), token_len,sql);
			start += token_len;
		}
		return std::make_unique<std::vector<token>>(tokens);
	}

	void token::print() const{
		printf("type:%d offset:%d len:%d %s\n", (int)type, offset, len,std::string(sql,offset,len).c_str());
	}

};