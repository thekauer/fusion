#pragma once
#include "lex.h"


enum class Parse_e {
	FnDecl,
	FnCall,
	Var_Decl,
	For_in,
	For_times,
	If,
	If_in,
	Else,
	While,
	Loop, // for infinite loops
	Switch,
	Expr,
	Range,
	Generator,
};


struct Parse_t {
	Parse_e type;
	Parse_t(Parse_e type);
};




class Parser {
	std::vector<token_t>& tokens;
public:
	Parser(std::vector<token_t>& tokens);
	void parse();
};