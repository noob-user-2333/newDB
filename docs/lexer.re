//#include "token.h"
namespace iedb {

    static token_type lexer_token(const char * start,const char *end,int& len){
         const char * YYCURSOR = start;
         const char * YYMARKER = YYCURSOR;
         const char * YYLIMIT = end;
         token_type type = token_type::error;
         /*!re2c
         re2c:define:YYCTYPE = char;
         re2c:yyfill:enable = 0;

         "," {type = token_type::comma; len = static_cast<uint32>(YYCURSOR - start);return type;}
         ";" {type = token_type::semi; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "." {type = token_type::dot; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "(" {type = token_type::parenthesis_left; len = static_cast<uint32>(YYCURSOR - start);return type;}
         ")" {type = token_type::parenthesis_right; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "[" {type = token_type::bracket_left; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "]" {type = token_type::bracket_right; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "{" {type = token_type::brace_left; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "}" {type = token_type::brace_right; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "+" {type = token_type::plus; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "-" {type = token_type::minus; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "*" {type = token_type::star; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "/" {type = token_type::slash; len = static_cast<uint32>(YYCURSOR - start);return type;}
         ">="{type = token_type::more_equal; len = static_cast<uint32>(YYCURSOR - start);return type;}
         ">" {type = token_type::more; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "<" {type = token_type::less; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "<=" {type = token_type::less_equal; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "!=" {type = token_type::not_equal; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "!" {type = token_type::bang; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "=" {type = token_type::equal; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "|" {type = token_type::bit_or; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "&" {type = token_type::bit_and; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "not" {type = token_type::Not; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "and" {type = token_type::And; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "or" {type = token_type::Or; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "select" {type = token_type::select; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "create" {type = token_type::create; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "update" {type = token_type::update; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "delete" {type = token_type::Delete; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "from" {type = token_type::from; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "where" {type = token_type::where; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "order" {type = token_type::order; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "group" {type = token_type::group; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "by" {type = token_type::by; len = static_cast<uint32>(YYCURSOR - start);return type;}
         [ \t\r\n]+ {type = token_type::space; len = static_cast<uint32>(YYCURSOR - start);return type;}
         [0-9]+"."[0-9]+ {type = token_type::number_float; len = static_cast<uint32>(YYCURSOR - start);return type;}
         [0-9]+ {type = token_type::number_int; len = static_cast<uint32>(YYCURSOR - start);return type;}
         "'"[^']*"'" {type = token_type::string; len = static_cast<uint32>(YYCURSOR - start);return type;}
         [_a-zA-Z][_0-9a-zA-Z]*    {type = token_type::name; len = static_cast<uint32>(YYCURSOR - start);return type;}
         '0x'[0-9a-fA-F]+ {type = token_type::hex; len = static_cast<uint32>(YYCURSOR - start);return type;}
         * {printf("something wrong start at %s\n",start);type = token_type::error;len = 1;return type;}

         */
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

	void token::print() {
		printf("type:%d offset:%d len:%d %s\n", type, offset, len,std::string(sql,offset,len).c_str());
	}

};